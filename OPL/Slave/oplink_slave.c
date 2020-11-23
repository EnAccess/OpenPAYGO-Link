/*
 * Filename:    oplink_slave.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/04/2020
 * Description: Slave node functions.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "byte_utils.h"
#include "oplink_com.h"
#include "oplink_com_private.h"
#include "oplink_slave.h"
#include "oplink_slave_private.h"

static bool opl_bus_locked = false;

opl_slave_t opl_slave = {Disconnected, false, 0, {0}};

static uint32_t nonce;

static struct {
    uint8_t plug_in;
    uint8_t disconnect;
    uint8_t join_net;
    uint8_t no_config;
    uint16_t no_ping;
} counter;

static uint8_t load_config() {
    opl_slave.mode = OPL_LOAD_MODE();
    if(opl_slave.mode == NO_CONFIG) return LOAD_MODE_ERROR;

    uint32_t seed = OPL_LOAD_SEED();
    if(seed == 0) return LOAD_SEED_ERROR;
    srand(seed);

    memset(opl_slave.uid, 0, UID_SIZE + 1);
    if(opl_slave.mode < HAS_UID) return LOAD_SUCCESS; // No UID

    OPL_LOAD_UID(opl_slave.uid);
    if(opl_slave.uid[0] == 0) return LOAD_UID_ERROR;

    return LOAD_SUCCESS;
}

static void slave_set_default() {
    OPL_UART_DISABLE_RX();
    opl_node_set_addr(0x00); // Don't change with RX enabled

    opl_slave.net_state = Disconnected;
    opl_slave.received_ping = false;
    memset(&counter, 0, sizeof(counter));

    // Set UART to 9-N-1 mode @ 9600 baud with addr 0x00
    OPL_UART_INIT(DEFAULT_ADDR, uart_rx_callback);

    OPL_DELAY(1);
}

static void check_physical_connection() {
    if(OPL_READ_RX_PIN()) {
        counter.disconnect = 0;
        if(opl_slave.net_state == Disconnected) {
            if(++counter.plug_in == PLUG_IN_MAX_COUNT){
                counter.plug_in = 0;
                opl_slave.net_state = Plugged_in;
                counter.join_net = (uint8_t)(rand() % 40) + 1;
                OPL_UART_CLEAR_BUSY();
                OPL_UART_FLUSH_RX();
            }
        }
    }
    else{
        counter.plug_in = 0;
        if(opl_slave.net_state != Disconnected) {
            if(++counter.disconnect == DISCONNECT_MAX_COUNT) {
                counter.disconnect = 0;
                slave_set_default();
            }
        }
    }
}

static void join_network() {
    if(counter.join_net > 0) {
        if(OPL_UART_IS_BUSY()) {
            counter.join_net = (uint8_t)(rand() % 40) + 1;
            OPL_UART_CLEAR_BUSY();
            OPL_UART_FLUSH_RX();
        }
        else{
            counter.join_net--;
            if(counter.join_net == 0){
                nonce = opl_hton32(rand());
                // Put the version in the first byte
                nonce = (nonce & 0x0FFF) | (uint32_t)(HSK_VER) << 24;
                opl_send_cmd(MASTER_ADDR, SIGNAL, &nonce, 4);
                opl_slave.net_state = Signal_sent;
            }
        }
    }
}

static void check_handshake() {
    if(++counter.no_config == NO_CONFIG_MAX_COUNT) {
        slave_set_default();
    }
}

static void check_ping() {
    if(opl_slave.received_ping) {
        opl_slave.received_ping = false;
        counter.no_ping = 0;
    }
    else if(++counter.no_ping == NO_PING_MAX_COUNT) {
        slave_set_default();
    }
}

void opl_init(){
    if(load_config() != LOAD_SUCCESS) {
        while(1) {
            // Not configured
        }
    }

    OPL_LIN_INIT(); // Configure write enable pin

    slave_set_default();

    OPL_LIN_DISABLE_TX();
}

void new_message_callback(){
    opl_bus_locked = false; // Unlock the bus when a request is received
}

/* Route the OPLink internal commands */
void route_command(uint8_t *buf, uint8_t len) {
    switch(buf[0]) { // Cmd byte
        case FIND: // FIND(1B), NONCE(4B), ADDR(1B)
            if(opl_slave.net_state == Signal_sent) {
                // Check if the nonce matches
                if(memcmp(&nonce, (buf + 1), 4) == 0){
                    OPL_UART_DISABLE_RX(); // send_bytes() re-enables rx later
                    opl_node_set_addr(buf[5]); // Don't change with RX enabled
                    opl_send_cmd(SOURCE_ADDR, ACK, NULL, 0);
                    opl_slave.net_state = Addr_set;
                }
            }
            break;
        case GET_UID:
            if(opl_slave.net_state == Addr_set)
                opl_slave.net_state = UID_sent;
            opl_send_cmd(SOURCE_ADDR, ACK, opl_slave.uid,
                         strlen(opl_slave.uid)); // If no UID strlen() = 0;
            break;
        case PING:
            if(opl_slave.net_state == UID_sent)
                opl_slave.net_state = Connected;
            opl_send_cmd(SOURCE_ADDR, ACK, NULL, 0);
            opl_slave.received_ping = true;
            break;

    }
}

/* This action is triggered by a trusted source, either when sending a presence
 * signal or when it is called from route_command(). In both cases the server
 * knows that it's safe to send something, the presence signal is only sent when
 * the bus is idle, and route_command() is called from opl_parse() which in fact
 * is the one freeing up the bus resource. Thus no need to check opl_bus_locked.
 */
bool opl_send_cmd(uint8_t addr, uint8_t cmd, uint8_t *args, uint8_t len) {
    uint8_t buffer[CMD_MAX_LEN];

    if(len > CMD_MAX_LEN - 1) return false;

    buffer[0] = cmd;
    if(args != NULL) memcpy(buffer + 1, args, len);
    opl_send_bytes(addr, CMD, buffer, len + 1);

    opl_bus_locked = true; // Here lock the resource, a reply is always preceded
                           // by a request. This way we make sure other modules
                           // don't send when they are not supposed.
    return true;
}

void opl_send_reply(uint8_t *data, uint8_t len) {
    if(opl_bus_locked == false){
        opl_send_bytes(SOURCE_ADDR, DATA, data, len);
        opl_bus_locked = true; // Here lock the resource, a reply is always
                               // preceded by a request. This way we make sure
                               // other modules don't send when they are not
                               // supposed.
    }
}

void opl_keep_alive() {
    static uint32_t old_millis = 0;

    if(OPL_MILLIS() - old_millis > LOOP_TIME) {
        old_millis = OPL_MILLIS();

        busy_timeout();
        check_physical_connection();

        switch(opl_slave.net_state) {
            case Plugged_in:
                join_network();
                break;
            case Signal_sent:
                // fallthrough
            case Addr_set:
                // fallthrough
            case UID_sent:
                check_handshake();
                break;
            case Connected:
                check_ping();
                break;
        }
    }
}
