/*
 * Filename:    byte_utils.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  19/05/2020
 * Description: Endianness helper functions.
 */

#ifndef BYTE_UTILS_H
#define BYTE_UTILS_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#if ! defined BIG_ENDIAN && ! defined LITTLE_ENDIAN
#error #error "Endianess not defined"
#endif

#define DEC 10
#define HEX 16

uint16_t opl_hton16(const uint16_t host);

uint32_t opl_hton32(const uint32_t host);

inline uint16_t opl_ntoh16(const uint16_t network){
    return opl_hton16(network);
}

inline uint32_t opl_ntoh32(const uint32_t network){
    return opl_hton32(network);
}

#ifdef __cplusplus
}
#endif

#endif /* BYTE_UTILS_H */
