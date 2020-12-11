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

#define CYCLES_PER_SECOND 20 // 1000 / LOOP_TIME

static uint8_t new_slave_addr;

bool opl_push_request(uint8_t *uid, uint8_t *data, uint8_t len) {
    uint8_t dest = _map_uid_to_addr(uid);
    if(dest == 0) return false; // No uid match
    return push_request(dest, data, len, true); // Wait for reply
}

bool opl_push_broadcast(uint8_t *data, uint8_t len) {
    return push_request(0x00, data, len, false); // Don't wait for reply
}

static void handle_new_slave(uint8_t *args) {
    new_slave_addr = slave_list_available();
    uint8_t temp_buf[5];
    if(new_slave_addr) {
        switch(args[0]) { // Test the version
            case 0x01:
                memcpy(temp_buf, args + 1, 4);
                temp_buf[4] = new_slave_addr;
                opl_send_cmd(DEFAULT_ADDR, FIND, temp_buf, 5, true, true);
                break;
        }
    }
}

static void handle_ack(uint8_t *args, uint8_t len) {
    uint8_t addr_buffer;
    switch(get_last_cmd()) {
        case FIND:
            opl_send_cmd(new_slave_addr, GET_UID, NULL, 0, true, true);
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

                opl_send_cmd(new_slave_addr, PING, NULL, 0, true, true);
            }
            break;
        case PING:
            slave_set_ping_period(get_last_dest(), 5);
            break;
    }
}

void opl_init() {
    OPL_LIN_INIT(); // Configure write enable pin
    OPL_LIN_ENABLE_TX();

    opl_node_set_addr(MASTER_ADDR);
    slave_list_init();
    request_queue_init();
    // Set UART to 9-N-1 mode @ 19200 baud with addr 0x0F
    opl_node_set_addr(MASTER_ADDR);
    OPL_UART_INIT(MASTER_ADDR, uart_rx_callback);
}

/* Route the OPLink internal commands. This is called form opl_parse(). It might
 * seem unsafe, but ACK is received only after an internal request, and SIGNAL
 * will only be sent by a slave that made sure that the bus is idle.
 */
void route_command(uint8_t *buf, uint8_t len) {
    //if(last_request.reply != pending) {
        switch(buf[0]){
            case SIGNAL:
                handle_new_slave(buf + 1);
                break;
            case ACK:
                handle_ack(buf + 1, --len); // We know len is at least 1
                break;
        }
    //}
}

void opl_keep_alive() {
    static uint32_t old_millis = 0;
    static uint8_t seconds = 0;

    if(OPL_MILLIS() - old_millis > LOOP_TIME) {
        old_millis = OPL_MILLIS();

        if(seconds++ == CYCLES_PER_SECOND) {
            seconds = 0;
            slave_list_ping_tick(); // Just tick
        }

        if(update_node_state() == RECEIVE_TIMEOUT_ERROR) {
            // Increase slave ping error if it didn't reply to the PING
            if(get_last_cmd() == PING)
                slave_ping_error(get_last_dest());
        }

        if(safe_to_send()) {
            // Ping has higher priority so it is checked first
            uint8_t ping_addr = next_slave_ping();

            if(ping_addr > 0) // Addresses start from 1, 0 means no need to ping
                opl_send_cmd(ping_addr, PING, NULL, 0, true, false);
            else // Dispatch next request if any
                dispatch_request();
        }
    }
}
