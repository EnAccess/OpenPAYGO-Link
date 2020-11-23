/*
 * Filename:    crc16.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  05/05/2020
 * Description: CRC-16/CCITT-FALSE implementation. More info: crccalc.com
 */

#ifndef CRC16_H
#define CRC16_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CRC_POLY    0x1021
#define CRC_INIT    0xFFFF

uint16_t update_crc16(uint16_t crcValue, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* CRC16_H */
