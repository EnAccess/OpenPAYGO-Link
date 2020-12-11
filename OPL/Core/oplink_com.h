/*
 * Filename:    oplink_com.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  26/03/2020
 * Description: OPLink COM public functions.
 */

#ifndef OPLINK_COM_H
#define OPLINK_COM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Check if there is a frame ready to be read and parse the header in that case.
 * Returns the number of received bytes (0-124). When the function is called,
 * it clears all the unread bytes from the last call. */
uint8_t opl_parse();

/* Read the desired number of received bytes, and return true if the CRC is OK.
 * The "len" parameter can be less than what opl_parse() returns, but then
 * several calls are needed to calculate properly the CRC. This function should
 * be called, right after calling opl_parse() and in the same loop iteration. */
bool opl_read(uint8_t *buf, uint8_t len);

/* Send a reply to a request. It must be called only after opl_read() returns
 * true. The function returns true if it was possible to send the reply or false
 * otherwise. */
bool opl_send_reply(uint8_t *buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* OPLINK_COM_H */
