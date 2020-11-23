/*
 * Filename:    crc16.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  05/05/2020
 * Description: CRC-16/CCITT-FALSE implementation. More info: crccalc.com
 */

#include "crc16.h"

uint16_t update_crc16(uint16_t crcValue, uint8_t byte) {
    for(uint8_t i = 0; i < 8; i++) {
        if( ((crcValue & 0x8000) >> 8) ^ (byte & 0x80) ) {
            crcValue = (crcValue << 1) ^ CRC_POLY;
        }
        else {
            crcValue = (crcValue << 1);
        }
    byte <<= 1;
    }
    return crcValue;
}
