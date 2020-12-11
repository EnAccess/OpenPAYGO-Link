/*
 * Filename:    oplink_com.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  26/03/2020
 * Description: OPLink COM core functions.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
//#include <stdlib.h>
#include "crc16.h"
#include "byte_utils.h"
#include "oplink_common.h"
#include "oplink_com.h"
#include "oplink_com_private.h"

#ifdef MASTER
#define BUS_WAIT() 0
#elif SLAVE
#include <stdlib.h>
#define BUS_WAIT() (uint8_t)(rand() % 20) + 1
#endif

/* Node information ***********************************************************/
static struct {
    uint8_t addr;
    bool bus_busy;
} opl_node = {0x00, true};
/******************************************************************************/

/* Received frame information *************************************************/
typedef enum {Empty, Ready, Processing} rx_frame_state_t;

static struct {
    rx_frame_state_t state;
    uint8_t busy_time;
    uint8_t src : 4;
    uint8_t dest : 4;
    uint8_t mode : 1;
    uint8_t len : 7;
    uint16_t crc;
} rx_frame = {Empty, SEND_REPLY_TIMEOUT, 0, 0, 0};
/******************************************************************************/

/* Last sent request information **********************************************/
typedef enum {None, Pending, Received} reply_state_t;

static struct {
    reply_state_t reply_state;
    uint8_t busy_time;
    uint8_t dest;
    uint8_t cmd;
} last_request = {None, RECEIVE_REPLY_TIMEOUT, 0xFF, 0xFF};
/******************************************************************************/

/* External request queue structs *********************************************/
#define MAX_REQUESTS 5 // Limited only by the memory available

typedef struct {
    uint8_t dest;
    uint8_t *buf; // Just a pointer
    uint8_t len;
    bool wait_reply;
} request_t;

static struct {
    request_t *head;
    request_t *tail;
    uint8_t count;
    request_t elems[MAX_REQUESTS];
} request_queue;
/******************************************************************************/

/* Low level UART interface functions *****************************************/
void opl_node_set_addr(uint8_t new_addr) {
    new_addr &= 0x0F;
    opl_node.addr = new_addr;
    OPL_UART_SET_ADDR(new_addr);
}

void uart_rx_callback(uint8_t b) {
    static uint8_t len = 0xFF;
    static uint8_t count;

    if(OPL_UART_IS_ADDR()) {
        len = 0xFF;
        count = 1; // First byte of the frame
    }
    else {
        count++;
        if(count == 2) {
            len = (b & 0x7F) + OVERHEAD; // len + header + footer
        }
        else if(count == len) {
            OPL_UART_MUTE(); // Mute here until next addr byte matches
            OPL_UART_DISABLE_RX(); // Only one frame at a time can be processed
            rx_frame.state = Ready;
            rx_frame.busy_time = SEND_REPLY_TIMEOUT;
        }
    }
}

/* Read the specified amount of bytes from the UART buffer and compute the CRC*/
static uint16_t opl_read_bytes(uint16_t crc, uint8_t *buf, uint8_t len) {
    uint8_t byte;

    for(uint8_t i = 0; i < len; i++) {
        byte = OPL_UART_READ_BYTE();
        crc = update_crc16(crc, byte);
        if(buf != NULL) buf[i] = byte; // Compute the CRC but don't store
    }

    return crc;
}

bool opl_send_bytes(uint8_t dest, frame_mode_t mode, uint8_t *data,
                    uint8_t len, bool force_write) {

    uint8_t result = false;
    uint8_t addr = ((opl_node.addr << 4) & 0xF0) | (dest & 0x0F); // src/dest
    uint8_t meta = (mode << 7) | len;

    #ifdef SLAVE
    OPL_LIN_ENABLE_TX(); // Set LIN transceiver to Operation Mode
    #endif /* SLAVE */

    // Write only if forced (sending a reply) or if the bus is not busy
    // This is the last place before writing where we can avoid a collision
    if(force_write == true || OPL_UART_IS_BUSY() == false) {
        OPL_UART_DISABLE_RX();  // Disable RX so you don't read your own data

        OPL_UART_WRITE_BREAK(); // Otherwise LIN transceiver misses first byte
        OPL_UART_WRITE_BYTE(SYNC_BYTE);

        uint16_t crc = CRC_INIT;

        crc = update_crc16(crc, addr);
        OPL_UART_WRITE_ADDR(addr);

        crc = update_crc16(crc, meta);
        OPL_UART_WRITE_BYTE(meta);

        for(uint8_t i = 0; i < len; i++) {
            crc = update_crc16(crc, data[i]);
            OPL_UART_WRITE_BYTE(data[i]);
        }

        crc = opl_hton16(crc); // Convert to network (big) endianness

        OPL_UART_WRITE_BYTE((uint8_t)(crc >> 8)); // First CRC byte, MSB
        OPL_UART_WRITE_BYTE((uint8_t)(crc & 0x00FF)); // Second CRC byte, LSB

        result = true;
    }

    #ifdef SLAVE
    OPL_LIN_DISABLE_TX();
    #endif /* SLAVE */

    OPL_UART_ENABLE_RX(); // Recover as soon as possible
    rx_frame.state = Empty; // Reset the rx frame state

    return result;
}
/******************************************************************************/

/* Auxiliary communication functions ******************************************/
node_state_t update_node_state() {
    node_state_t result = NODE_OK;

    if(rx_frame.state != Empty && --rx_frame.busy_time == 0) {
        UART_ENABLE_RX();
        rx_frame.state = Empty;
        result = SEND_TIMEOUT_ERROR;
    }

    if(last_request.reply_state == Pending && --last_request.busy_time == 0) {
        last_request.reply_state = None;
        result = RECEIVE_TIMEOUT_ERROR; // Higher priority
    }

    static uint8_t busy_count = 0;
    if(busy_count == 0) {
        opl_node.bus_busy = OPL_UART_IS_BUSY();
        if(opl_node.bus_busy) {
            OPL_UART_CLEAR_BUSY();
            busy_count = BUS_WAIT();
        }
    }
    else busy_count--;

    return result;
}

bool safe_to_send() {
    return (rx_frame.state == Empty) && (last_request.reply_state == None) &&
           (opl_node.bus_busy == false);
}

uint8_t get_last_dest() {
    return last_request.dest;
}

uint8_t get_last_cmd() {
    return last_request.cmd;
}
/******************************************************************************/

/* Internal communication functions *******************************************/
bool opl_send_cmd(uint8_t addr, uint8_t cmd, uint8_t *args, uint8_t len,
                  bool wait_reply, bool force_write) {
    uint8_t buffer[CMD_MAX_LEN];

    if(len > CMD_MAX_LEN - 1) return false;

    buffer[0] = cmd;
    if(args != NULL) memcpy(buffer + 1, args, len);

    if(opl_send_bytes(addr, CMD, buffer, len + 1, force_write)) {
        if(wait_reply) {
            last_request.reply_state = Pending;
            last_request.busy_time = RECEIVE_REPLY_TIMEOUT;
            last_request.dest = addr & 0x0F;
            last_request.cmd = cmd;
        }
        return true;
    }

    return false;
}

extern void route_command(uint8_t *buf, uint8_t len);
/******************************************************************************/

/* High level communication functions *****************************************/
uint8_t opl_parse() {
    uint8_t result = RX_NOT_READY;

    if(rx_frame.state == Ready) {
        rx_frame.state = Processing;

        uint8_t byte; // Bitwise on arrays was causing issues
        rx_frame.crc = opl_read_bytes(CRC_INIT, &byte, 1);
        rx_frame.src = byte >> 4; // First 4 bits
        rx_frame.dest = byte & 0x0F; // Remaining 4 bits

        // Keep proccessing if we are not waiting for a reply or if we are
        // waiting for a reply and we received a message from the requested node
        switch(last_request.reply_state) { // Idea: expand this to return errors
            case Pending:
                if(last_request.dest != rx_frame.src) {
                    OPL_UART_ENABLE_RX();
                    rx_frame.state = Empty;
                    break; // Exit the switch case
                }
                else {
                    last_request.reply_state = Received;
                    // fallthrough to the next case
                }
            case None:
                rx_frame.crc = opl_read_bytes(rx_frame.crc, &byte, 1);
                rx_frame.mode = byte >> 7; // First bit
                rx_frame.len = byte & 0x7F; // Remaining 7 bits

                if(rx_frame.len > 0 && rx_frame.mode == CMD) {
                    uint8_t tmp_buf[CMD_MAX_LEN];
                    uint8_t len = rx_frame.len; // Save the len before reading
                    if(opl_read(tmp_buf, rx_frame.len)) {
                        route_command(tmp_buf, len);
                    }
                    result = NO_BYTES;
                }
                else {
                    result = rx_frame.len;
                }
                break;
        }
    }
    return result;
}

bool opl_read(uint8_t *buf, uint8_t len) {
    bool crc_ok = false;

    if(len > rx_frame.len) len = rx_frame.len;

    rx_frame.crc = opl_read_bytes(rx_frame.crc, buf, len);

    rx_frame.len -= len;
    if(rx_frame.len == 0) { // All the data bytes were read
        // Read the CRC and test if it is correct
        crc_ok = ( 0x0000 == opl_read_bytes(rx_frame.crc, NULL, CRC_LEN) );
        if(( crc_ok == false) || (last_request.reply_state == Received) ) {
            last_request.reply_state = None;
            OPL_UART_ENABLE_RX();
            rx_frame.state = Empty;
        }
        // To prevent the node getting stuck if the buffer was not fully read
        // nor the reply was sent, after a timeout the UART RX will be reenabled
    }
    return crc_ok;
}

/* Used only for DATA type frames sent by the application layer. */
bool opl_send_reply(uint8_t *buf, uint8_t len) {
    if(rx_frame.state != Processing || rx_frame.mode != DATA) return false;

    opl_send_bytes(rx_frame.src, DATA, buf, len, true); // Reply to the source
    return true;
}
/******************************************************************************/

/* External request queue functions *******************************************/
void request_queue_init() {
    request_queue.head = request_queue.elems;
    request_queue.tail = request_queue.elems;
    request_queue.count = 0;
}

bool push_request(uint8_t dest, uint8_t *data, uint8_t len, bool wait_reply) {
    if(request_queue.count < MAX_REQUESTS - 1) {
        request_queue.head->dest = dest;
        request_queue.head->buf = data;
        request_queue.head->len = len;
        request_queue.head->wait_reply = wait_reply;

        request_queue.count++;
        if(request_queue.head - request_queue.elems == MAX_REQUESTS - 1)
            request_queue.head = request_queue.elems; // First element
        else
            request_queue.head++;

        return true;
    }
    else
        return false;
}

void dispatch_request() {
    if(request_queue.count > 0) {
        if(opl_send_bytes(request_queue.tail->dest, DATA,
                          request_queue.tail->buf, request_queue.tail->len,
                          false)) { // If it was possible to send then clear

            if(request_queue.tail->wait_reply) {
                last_request.reply_state = Pending;
                last_request.busy_time = RECEIVE_REPLY_TIMEOUT;
                last_request.dest = request_queue.tail->dest;
                last_request.cmd = EXT; // External request dummy command
            }

            request_queue.count--;
            if(request_queue.tail - request_queue.elems == MAX_REQUESTS - 1)
                request_queue.tail = request_queue.elems; // First element
            else
                request_queue.tail++;
        }
    }
}
/******************************************************************************/
