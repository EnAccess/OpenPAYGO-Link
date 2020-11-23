/*
 * Filename:    oplink_com_private.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  07/08/2020
 * Description: OPLink COM private functions.
 */

#ifndef OPLINK_COM_PRIVATE_H
#define OPLINK_COM_PRIVATE_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "oplink_common.h"

#define LOOP_TIME 50U //50ms
#define BUSY_TIMEOUT 4U // 500 ms
#define RESPONSE_TIMEOUT 2U

void uart_rx_callback(uint8_t b);

void busy_timeout();

void opl_node_set_addr(uint8_t new_addr);

void opl_send_bytes(uint8_t dest, frame_mode_t mode, uint8_t *data,
                    uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* OPLINK_COM_PRIVATE_H */
