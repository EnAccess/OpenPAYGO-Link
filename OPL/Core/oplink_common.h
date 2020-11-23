/*
 * Filename:    oplink_common.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/03/2020
 * Description: Common OPL definitions.
 */

#ifndef COMMON_H
#define COMMON_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "oplink_adapters.h"

#define HSK_VER 0x01 // Handshake version
#define MAX_SLAVES  5

#define HEADER_LEN  2
#define CRC_LEN     2
#define OVERHEAD 4 // HEADER_LEN + CRC_LEN
#define CMD_MAX_LEN 16
#define OPL_PAYLOAD_MAX_LEN 124 // 128 byte UART buffer - 4 OVERHEAD

#define MASTER_ADDR  0x0F
#define DEFAULT_ADDR 0x00
#define SOURCE_ADDR  0xF0

#define SYNC_BYTE 0x55

typedef enum {
    DATA = 0,
    CMD  = 1
} frame_mode_t;

enum parse_result {
    RX_NOT_READY = 0,
    NO_BYTES     = 0
};

enum core_commands {
    SIGNAL   = 0,
    FIND     = 1,
    GET_UID  = 2,
    PING     = 3,
    ALERT    = 4,
    ACK      = 6,  // ASCII
    NACK     = 15, // ASCII
    EXT      = 20  // Dummy command
};

enum handshake_error {
    NULL_POINTER_ERROR  = -1,
    REPLY_TIMEOUT_ERROR = -2,
    BAD_CRC_ERROR       = -3
};

#define IPv6_SIZE 6
#define UID_SIZE  12
#define KEY_SIZE  8

enum slave_mode {
    NO_CONFIG  = 0,
    NO_UID     = 1,
    HAS_UID    = 2
};

enum load_state {
    LOAD_SUCCESS,
    LOAD_MODE_ERROR,
    LOAD_SEED_ERROR,
    LOAD_UID_ERROR,
};

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
