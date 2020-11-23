/*
 * Filename:    byte_utils.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  19/05/2020
 * Description: Endianness helper functions.
 */

#include "byte_utils.h"

uint16_t opl_hton16(const uint16_t host) {
    #ifdef BIG_ENDIAN
        return host;
    #else
        uint16_t network = 0;
        ((uint8_t*) &network)[0] = host & 0xFF;
        ((uint8_t*) &network)[1] = (host & 0x0000FF00) >> 8;
        return network;
    #endif
}

uint32_t opl_hton32(const uint32_t host) {
    #ifdef BIG_ENDIAN
        return host;
    #else
        uint32_t network = 0;
        ((uint8_t*) &network)[0] = host >> 24;
        ((uint8_t*) &network)[1] = (host & 0x00FF0000) >> 16;
        ((uint8_t*) &network)[2] = (host & 0x0000FF00) >> 8;
        ((uint8_t*) &network)[3] = host;
        return network;
    #endif
}
