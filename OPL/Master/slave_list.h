/*
 * Filename:    slave_list.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/03/2020
 * Description: Slave management functions.
 */

#ifndef SLAVE_LIST_H
#define SLAVE_LIST_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "oplink_common.h"

/*
 * List that contains the number of connected devices and their UIDs.
 */
typedef struct {
    uint8_t n_slaves;
    uint8_t uids[MAX_SLAVES][UID_SIZE + 1];
} opl_slave_list_t;

/*
 * Fills the passed struct with the most updated network status: number of nodes
 * and UID of each node. It is recommended to call this function periodically.
 * TODO: add a network change (node connected/disconnected) callback.
 */
void get_slave_list(opl_slave_list_t *ptr);

#ifdef __cplusplus
}
#endif

#endif /* SLAVE_LIST_H */
