/*
 * Filename:    oplink_com.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  26/03/2020
 * Description: OPLink COM core functions.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "crc16.h"
#include "byte_utils.h"
#include "oplink_common.h"
#include "oplink_com.h"
#include "oplink_com_private.h"

typedef enum {Idle, Ready, Busy} state_t;

uint8_t last_dst = 0;

static struct {
    state_t state;
    uint8_t addr;
    uint8_t busy_time;
} opl_node = {Idle, 0x00, 0};

static struct {
    uint8_t src : 4;
    uint8_t dst : 4;
    uint8_t mode : 1;
    uint8_t len : 7;
    uint16_t crc;
} rx_frame = {0, 0, 0};

void uart_rx_callback(uint8_t b) {
    // A frame consists of a 2-byte header + n-byte payload + 2-byte CRC
    // The first byte of the header contains the src & dest addresses, the
    // second byte contains the message mode (cmd or data) represented as 1 bit,
    // and the payload length stored in the following 7 bits.

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
            opl_node.state = Ready;
            opl_node.busy_time = 0;
        }
    }
}

void busy_timeout() {
    if(opl_node.state != Idle && opl_node.busy_time++ == BUSY_TIMEOUT) {
        OPL_UART_ENABLE_RX();
        opl_node.state = Idle;
    }
}

void opl_node_set_addr(uint8_t new_addr) {
    new_addr &= 0x0F;
    opl_node.addr = new_addr;
    OPL_UART_SET_ADDR(new_addr);
}

void opl_send_bytes(uint8_t dest, frame_mode_t mode, uint8_t *data,
                    uint8_t len) {

    // Not sending is better than sending an incomplete message.
    if(len <= OPL_PAYLOAD_MAX_LEN) {
        if(dest == SOURCE_ADDR) dest = rx_frame.src;
        last_dst = ((opl_node.addr << 4) & 0xF0) | (dest & 0x0F); // src/dst
        uint8_t meta = (mode << 7) | len;

        #ifdef SLAVE
        OPL_LIN_ENABLE_TX(); // Set LIN transceiver to Operation Mode
        #endif /* SLAVE */

        OPL_UART_DISABLE_RX();  // Disable RX so you don't read your own data

        OPL_UART_WRITE_BREAK(); // Otherwise LIN transceiver misses first byte
        OPL_UART_WRITE_BYTE(SYNC_BYTE);

        uint16_t crc = CRC_INIT;

        crc = update_crc16(crc, last_dst);
        OPL_UART_WRITE_ADDR(last_dst);

        crc = update_crc16(crc, meta);
        OPL_UART_WRITE_BYTE(meta);

        for(uint8_t i = 0; i < len; i++) {
            crc = update_crc16(crc, data[i]);
            OPL_UART_WRITE_BYTE(data[i]);
        }

        crc = opl_hton16(crc); // Convert to network (big) endianness

        OPL_UART_WRITE_BYTE((uint8_t)(crc >> 8)); // First CRC byte, MSB
        OPL_UART_WRITE_BYTE((uint8_t)(crc & 0x00FF)); // Second CRC byte, LSB

        #ifdef SLAVE
        OPL_LIN_DISABLE_TX();
        #endif /* SLAVE */
    }

    OPL_UART_ENABLE_RX(); // Recover as soon as possible
    opl_node.state = Idle; // The node is now idle and can receive
}

/*
 * Read only the specified amount of bytes from the UART buffer and compute the
 * CRC.
 */
static uint16_t opl_read_bytes(uint16_t crc, uint8_t *buf, uint8_t len) {
    uint8_t byte;

    for(uint8_t i = 0; i < len; i++) {
        byte = OPL_UART_READ_BYTE();
        crc = update_crc16(crc, byte);
        if(buf != NULL) buf[i] = byte; // Compute the CRC but don't store
    }

    return crc;
}

/*
 * Read the specified bytes and 2 extra bytes for the CRC if the all the payload
 * was read. The CRC bytes are processed but not stored.
 */
bool opl_read(uint8_t *buf, uint8_t len) {
    bool crc_ok = false;

    if(len > rx_frame.len) len = rx_frame.len;

    rx_frame.crc = opl_read_bytes(rx_frame.crc, buf, len);

    rx_frame.len -= len;
    if(rx_frame.len == 0) { // All the data bytes were read
        // Read the CRC and test if it is correct
        crc_ok = ( 0x0000 == opl_read_bytes(rx_frame.crc, NULL, CRC_LEN) );
        // Enable the UART RX only if the CRC was wrong or if it is a broadcast
        // or a reply from a previous request, otherwise wait until we answer.
        if(crc_ok == false || (rx_frame.dst == 0x00 && opl_node.addr != 0x00) ||
            (rx_frame.src == last_dst) ) {
            OPL_UART_ENABLE_RX();
            opl_node.state = Idle;
        }
        // To prevent the node getting stuck if the buffer was not fully read
        // nor the reply was sent, after a timeout the UART RX will be reenabled
    }
    return crc_ok;
}

extern void new_message_callback();
extern void route_command(uint8_t *buf, uint8_t len);

uint8_t opl_parse() {
    uint8_t result = RX_NOT_READY;

    if(opl_node.state == Ready) {
        opl_node.state = Busy;

        // If we are here we already now that the destination address is correct
        uint8_t byte; // Bitwise on arrays was causing issues
        rx_frame.crc = opl_read_bytes(CRC_INIT, &byte, 1);
        rx_frame.src = byte >> 4; // First 4 bits
        rx_frame.dst = byte & 0x0F; // Remaining 4 bits

        new_message_callback();

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
    }

    return result;
}
