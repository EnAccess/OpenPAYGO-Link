/*
 * Filename:    oplink_slave_private.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/04/2020
 * Description: Slave node private functions.
 */

#ifndef OPLINK_SLAVE_PRIVATE_H
#define OPLINK_SLAVE_PRIVATE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "oplink_com_private.h"

#define PLUG_IN_MAX_COUNT 20 // 20 * 50 = 1000ms
#define DISCONNECT_MAX_COUNT 10 // 10 * 50 = 500ms
#define NO_CONFIG_MAX_COUNT 40 // 40 * 50 = 2000ms
#define NO_PING_MAX_COUNT 1200 // 1200 * 50 = 60000ms

typedef enum {
    Disconnected,
    Plugged_in,
    Signal_sent,
    Addr_set,
    UID_sent,
    Connected
} net_t;

typedef struct {
    net_t net_state;
    bool received_ping : 1;
    uint8_t mode;
    uint8_t uid[UID_SIZE + 1];
} opl_slave_t;

extern opl_slave_t opl_slave;

bool opl_send_cmd(uint8_t addr, uint8_t cmd, uint8_t *args, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* OPLINK_SLAVE_PRIVATE_H */
