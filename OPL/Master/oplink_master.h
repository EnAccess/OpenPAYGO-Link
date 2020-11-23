/*
 * Filename:    oplink_master.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  13/01/2020
 * Description: Master node functions.
 */

#ifndef OPLINK_MASTER_H
#define OPLINK_MASTER_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "oplink_com.h"
#include "slave_list.h"

typedef enum { none, pending, received, timedout} reply_t;

void opl_init();

void opl_keep_alive();

reply_t opl_reply_state();

bool opl_push_request(uint8_t *uid, uint8_t *data, uint8_t len);

bool opl_push_broadcast(uint8_t dest, uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* OPLINK_MASTER_H */
