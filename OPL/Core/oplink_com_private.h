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
#define SEND_REPLY_TIMEOUT 2U
//#define RESPONSE_TIMEOUT 2U
#define RECEIVE_REPLY_TIMEOUT 3U // Smaller than SEND_REPLY_TIMEOUT

/* Set the node address */
void opl_node_set_addr(uint8_t new_addr);

/* Callback from UART RX ISR */
void uart_rx_callback(uint8_t b);

typedef enum {NODE_OK, SEND_TIMEOUT_ERROR, RECEIVE_TIMEOUT_ERROR} node_state_t;
/* Check if the received frame was processed or the reply was received on time
 * and update the bus busy status*/
node_state_t update_node_state();

/* Return true if no frame needs processing, not waiting for a reply and the bus
 * is idle. Return false otherwise. */
bool safe_to_send();

/* Returns the last destination */
uint8_t get_last_dest();

/* Returns the last command */
uint8_t get_last_cmd();

/* Send a command type frame with the specified parameters. */
bool opl_send_cmd(uint8_t addr, uint8_t cmd, uint8_t *args, uint8_t len,
                  bool wait_reply, bool force_write);

/* Initialize the external request queue. */
void request_queue_init();

/* Push a request to the queue. Returns true on success and false otherwise. */
bool push_request(uint8_t dest, uint8_t *data, uint8_t len, bool wait_reply);

/* Send a request if the bus is idle. On success remove it from the list. */
void dispatch_request();
#ifdef __cplusplus
}
#endif

#endif /* OPLINK_COM_PRIVATE_H */
