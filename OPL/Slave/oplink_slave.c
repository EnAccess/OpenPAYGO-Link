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

#define PLUG_IN_MAX_COUNT 20 // 20 * 50 = 1000ms
#define DISCONNECT_MAX_COUNT 10 // 10 * 50 = 500ms
#define NO_CONFIG_MAX_COUNT 40 // 40 * 50 = 2000ms
#define NO_PING_MAX_COUNT 1200 // 1200 * 50 = 60000ms = 60 seconds

typedef enum {
    Disconnected,
    Plugged_in,
    Signal_sent,
    Addr_set,
    UID_sent,
    Connected
} bus_state_t;

typedef struct {
    uint8_t nonce_buffer[5]; // 1 byte for the version + 4 bytes for a uint32_t
    bus_state_t bus_state;
    bool received_ping;
    uint8_t mode;
    uint8_t uid[UID_SIZE + 1];
} opl_slave_t;

opl_slave_t opl_slave = {{0}, Disconnected, false, 0, {0}};

static struct {
    uint8_t plug_in;
    uint8_t disconnect;
    uint8_t no_config;
    uint16_t no_ping;
} counter;

static bool opl_bus_locked = false;

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

    *(uint32_t *)(opl_slave.nonce_buffer + 1) = (uint32_t)opl_hton32(rand());
    opl_slave.bus_state = Disconnected;
    opl_slave.received_ping = false;
    memset(&counter, 0, sizeof(counter));

    // Set UART to 9-N-1 mode @ 19200 baud with addr 0x00
    OPL_UART_INIT(DEFAULT_ADDR, uart_rx_callback); // This enables RX & TX

    OPL_DELAY(1);
}

static void check_bus_connection() {
    if(OPL_READ_RX_PIN()) {
        counter.disconnect = 0;
        if(opl_slave.bus_state == Disconnected) {
            if(++counter.plug_in == PLUG_IN_MAX_COUNT){
                counter.plug_in = 0;
                opl_slave.bus_state = Plugged_in;
            }
        }
    }
    else{
        counter.plug_in = 0;
        if(opl_slave.bus_state != Disconnected) {
            if(++counter.disconnect == DISCONNECT_MAX_COUNT) {
                counter.disconnect = 0;
                slave_set_default();
            }
        }
    }
}

static void join_bus() {
    if(safe_to_send())
        if(opl_send_cmd(MASTER_ADDR, SIGNAL, opl_slave.nonce_buffer, 5,
           false, false))
            opl_slave.bus_state = Signal_sent;
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
    if(load_config() != LOAD_SUCCESS)
        while(1) { /* Not configured */ }

    OPL_LIN_INIT(); // Configure write enable pin
    opl_slave.nonce_buffer[0] = HSK_VER;
    request_queue_init();
    slave_set_default();
    OPL_LIN_DISABLE_TX();
}

/* Route the OPLink internal commands */
void route_command(uint8_t *buf, uint8_t len) {
    switch(buf[0]) { // Cmd byte
        case FIND: // FIND(1B), NONCE(4B), ADDR(1B)
            if(opl_slave.bus_state == Signal_sent) {
                // Check if the nonce matches
                if(memcmp((opl_slave.nonce_buffer + 1), (buf + 1), 4) == 0){
                    // Send reply before changing the address
                    opl_send_cmd(MASTER_ADDR, ACK, NULL, 0, false, true);
                    OPL_UART_DISABLE_RX();
                    opl_node_set_addr(buf[5]); // Don't change with RX enabled
                    OPL_UART_ENABLE_RX();
                    opl_slave.bus_state = Addr_set;
                }
            }
            break;
        case GET_UID:
            if(opl_slave.bus_state == Addr_set)
                opl_slave.bus_state = UID_sent;
            opl_send_cmd(MASTER_ADDR, ACK, opl_slave.uid,
                         strlen(opl_slave.uid), false, true);
            break;
        case PING:
            if(opl_slave.bus_state == UID_sent)
                opl_slave.bus_state = Connected;
            opl_send_cmd(MASTER_ADDR, ACK, NULL, 0, false, true);
            opl_slave.received_ping = true;
            break;

    }
}

bool opl_push_request(uint8_t *data, uint8_t len) {
    return push_request(MASTER_ADDR, data, len, true);
}

void opl_keep_alive() {
    static uint32_t old_millis = 0;

    if(OPL_MILLIS() - old_millis > LOOP_TIME) {
        old_millis = OPL_MILLIS();

        check_bus_connection();
        update_node_state();

        switch(opl_slave.bus_state) {
            case Plugged_in:
                join_bus();
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

        if(safe_to_send())
            dispatch_request();
    }
}
