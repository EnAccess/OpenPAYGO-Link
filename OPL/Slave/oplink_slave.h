/*
 * Filename:    oplink_slave.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/04/2020
 * Description: Slave node public functions.
 */

#ifndef OPLINK_SLAVE_H
#define OPLINK_SLAVE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "oplink_common.h"
#include "oplink_com.h"

/*
 * Initialize the OPLink module:
 * Set LIN transceiver control pin to output push-pull and drive it low
 * Set UART to 9-N-1 mode @ 9000 baud with address wake up
 * Flush all the buffers and put the node in a reset state
 */
void opl_init();

/*
 * Update the slave flags according to the bus state:
 * Check if the bus is physically connected or not
 * Check if the bus is idle or not
 * Check if the slave was configured by the master after it was connected
 * Check if the ping is getting received in the expected intervals
 */
void opl_keep_alive();

/*
 * Send the desired number of bytes. The header will be added to the frame and
 * the CRC will be calculated and appended at the end.
 */
void opl_send_reply(uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* OPLINK_SLAVE_H */
