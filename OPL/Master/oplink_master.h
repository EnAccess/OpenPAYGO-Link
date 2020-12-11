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

/* Initialize the OPLink module:
 * Set LIN transceiver control pin to output push-pull and drive it high
 * Set UART to 9-N-1 mode @ 9000 baud with address wake up
 * Flush all the buffers and put the node in a reset state */
void opl_init();

/* Update the slave flags according to the bus state:
 * Check if the bus is physically connected or not
 * Check if the bus is idle or not
 * Check if the slave was configured by the master after it was connected
 * Send ping to the connected devices */
void opl_keep_alive();

/* Push a request to the queue. The request will be sent to the addr
 * corresponding to the provided uid as soon as the device is idle and the bus
 * is free. */
bool opl_push_request(uint8_t *uid, uint8_t *data, uint8_t len);

/* Push a request to the queue. The request will be sent to all the nodes as
 * soon as the device is idle and the bus is free. */
bool opl_push_broadcast(uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* OPLINK_MASTER_H */
