/*
 * Filename:    oplink_master.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  13/01/2020
 * Description: Master node functions.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "crc16.h"
#include "oplink_common.h"
#include "oplink_com.h"
#include "oplink_com_private.h"
#include "slave_list_private.h"
#include "oplink_master.h"

#define LOOP_TIME 50U // 50ms
#define CYCLES_PER_SECOND 20

static bool handshake_in_process = false;
static uint8_t new_slave_addr;

typedef struct {
    uint8_t addr;
    uint8_t *buf;
    uint8_t len;
    bool wait_reply;
} request_t;

#define MAX_REQUESTS 5

struct {
    request_t *head;
    request_t *tail;
    uint8_t count;
    request_t elems[MAX_REQUESTS];
} request_queue;

struct {
    uint8_t dst;
    uint8_t cmd;
    reply_t reply;
    uint8_t time;
} last_request = {0, EXT, none, 0};

static bool _push_request(uint8_t dest, uint8_t *data, uint8_t len, bool wr) {
    if(request_queue.count < MAX_REQUESTS - 1) {
        request_queue.tail->addr = dest;
        request_queue.tail->buf = data;
        request_queue.tail->len = len;
        request_queue.tail->wait_reply = wr;

        request_queue.count++;

        if(request_queue.tail - request_queue.elems == MAX_REQUESTS - 1)
            request_queue.tail == request_queue.elems; // First element
        else
            request_queue.tail++;

        return true;
    }
    return false;
}

bool opl_push_request(uint8_t *uid, uint8_t *data, uint8_t len) {
    uint8_t dest = _map_uid_to_addr(uid);
    if(dest == 0) return false; // No uid match
    return _push_request(dest, data, len, true); // Wait for reply
}

bool opl_push_broadcast(uint8_t dest, uint8_t *data, uint8_t len) {
    return _push_request(dest, data, len, false); // Don't wait for reply
}

static void _send_request() {
    opl_send_bytes(request_queue.head->addr, DATA, request_queue.head->buf,
                   request_queue.head->len);

    if(request_queue.head->wait_reply) {
        last_request.reply = pending;
    }

    last_request.dst = request_queue.head->addr;
    last_request.cmd = EXT; // Not sent, just for internal checks

    request_queue.count--;

    if(request_queue.head - request_queue.elems == MAX_REQUESTS - 1)
        request_queue.head == request_queue.elems; // First element
    else
        request_queue.head++;
}

/* This function is used only for internal commands. In the handshake all the
 * commands are sent sequentially, triggered by the reply to the previous
 * command. The handshake can be seen as a long request with multiple transfers.
 */
static bool opl_send_cmd(uint8_t addr, uint8_t cmd, uint8_t *args, uint8_t len){
    uint8_t buffer[CMD_MAX_LEN];
    if(len > CMD_MAX_LEN - 1) return false;

    buffer[0] = cmd;
    if(args != NULL) memcpy(buffer + 1, args, len);
    opl_send_bytes(addr, CMD, buffer, len + 1);

    last_request.dst = addr;
    last_request.cmd = cmd;
    last_request.reply = pending;
    return true;
}

static void handle_new_slave(uint8_t *args) {
    new_slave_addr = slave_list_available();
    uint8_t temp_buf[5];
    if(new_slave_addr) {
        switch(args[0]) { // Test the version
            case 0x01:
                OPL_DELAY(1);
                memcpy(temp_buf, args, 4);
                temp_buf[4] = new_slave_addr;
                opl_send_cmd(DEFAULT_ADDR, FIND, temp_buf, 5);
                handshake_in_process = true;
                break;
        }
    }
}

static void handle_ack(uint8_t *args, uint8_t len) {
    uint8_t addr_buffer;
    switch(last_request.cmd) {
        case FIND:
            opl_send_cmd(new_slave_addr, GET_UID, NULL, 0);
            break;
        case GET_UID:
            if(len <= UID_SIZE) {
                if(len != 0) {
                    slave_list_add(new_slave_addr, args, len);
                }
                else {
                    addr_buffer = new_slave_addr + '0';
                    slave_list_add(new_slave_addr, &addr_buffer, 1);
                }

                opl_send_cmd(new_slave_addr, PING, NULL, 0);
                handshake_in_process = false;
            }
            break;
        case PING:
            slave_set_ping_period(last_request.dst, 5);
            break;
    }
}

void opl_init() {
    OPL_LIN_INIT(); // Configure write enable pin
    OPL_LIN_ENABLE_TX();

    opl_node_set_addr(MASTER_ADDR);

    slave_list_init();

    request_queue.head = request_queue.elems;
    request_queue.tail = request_queue.elems;
    request_queue.count = 0;

    // Set UART to 9-N-1 mode @ 9600 baud with addr 0x0F
    opl_node_set_addr(MASTER_ADDR);
    OPL_UART_INIT(MASTER_ADDR, uart_rx_callback);
}

/* This function is called from opl_parse() when the frame is ready. sUpdate the
 * the last request fields only if we received a reply from the node which we
 * sent the request to.
 */
void new_message_callback() {
    last_request.reply = received;
    last_request.time = 0;
}

/* Route the OPLink internal commands. This is called form opl_parse(). It might
 * seem unsafe, but ACK is received only after an internal request, and SIGNAL
 * will only be sent by a slave that made sure that the bus is idle.
 */
void route_command(uint8_t *buf, uint8_t len) {
    if(last_request.reply != pending) {
        switch(buf[0]){
            case SIGNAL:
                handle_new_slave(buf + 1);
                break;
            case ACK:
                handle_ack(buf + 1, --len); // We know len is at least 1
                break;
        }
    }
}

void opl_keep_alive() {
    static uint32_t old_millis = 0;
    static uint8_t seconds = 0;

    if(OPL_MILLIS() - old_millis > LOOP_TIME) {
        old_millis = OPL_MILLIS();

        busy_timeout();

        if(seconds++ == CYCLES_PER_SECOND) {
            seconds = 0;
            slave_list_ping_tick(); // Just tick
        }

        // Still waiting for reply, cannot write
        if(last_request.reply == pending){
            if(last_request.time++ == RESPONSE_TIMEOUT) {
                // Reply time expired and it is safe to send a new request
                last_request.reply = timedout;
                last_request.time = 0;
                // Terminate the handshake process if needed
                if(handshake_in_process)
                    handshake_in_process = false;
                // Remove slave if it didn't reply to the PING
                if(last_request.cmd == PING)
                    slave_ping_error(last_request.dst);
            }
        }
        // If there's no ongoing handshake then it is safe to write
        else if(handshake_in_process == false) {
            // Ping has higher priority so it is checked first
            uint8_t ping_addr = next_slave_ping();

            if(ping_addr) // Addresses start from 1, 0 means no need to ping
                opl_send_cmd(ping_addr, PING, NULL, 0);

            else if(request_queue.count > 0) // Dispatch next request if any
                _send_request();
        }
    }
}

reply_t opl_reply_state() {
    if(last_request.cmd == EXT)
        return last_request.reply;
    else
        return none;
}
