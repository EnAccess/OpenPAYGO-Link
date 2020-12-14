/*
 * Filename:    slave_list_private.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/03/2020
 * Description: Slave management functions.
 */

#ifndef SLAVE_LIST_PRIVATE_H
#define SLAVE_LIST_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include "oplink_common.h"

#define MAX_PING_ERROR 3
#define PING_PERIOD 30 // seconds

typedef struct {
    uint8_t addr;
    uint8_t uid[UID_SIZE];
    uint8_t ping_count;
    uint8_t ping_error;
} slave_t;

void slave_list_init();

uint8_t slave_list_available();

bool slave_list_add(uint8_t new_addr, uint8_t *uid, uint8_t len);

void slave_clear_slot(uint8_t addr);

void slave_ping_error(uint8_t addr);

void slave_set_ping_period(uint8_t addr, uint8_t ticks);

void slave_list_ping_tick();

uint8_t next_slave_ping();

uint8_t map_uid_to_addr(uint8_t *uid);

#endif /* SLAVE_LIST_PRIVATE_H */
