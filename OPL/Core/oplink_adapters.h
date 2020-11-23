/*
 * Filename:    opl_adapters.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  05/05/2020
 * Description: Adaper layer to map target specifc functions to standard OPL.
 */

#ifndef OPL_ADAPTERS_H
#define OPL_ADAPTERS_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "oplink_common.h"

////////////////////////////  I N T E R R U P T S  /////////////////////////////

#include "stm8s.h"
#define OPL_ENABLE_INTERRUPTS()     enable_interrupts()
#define OPL_DISABLE_INTERRUPTS()    disable_interrupts()


/////////////////////////////////  D E L A Y  //////////////////////////////////

#include "delay.h"
#define OPL_DELAY(_ms)   delay_ms(_ms)


/////////////////////////////////  T I M E R  //////////////////////////////////

#include "timer.h"
#define OPL_MILLIS()                        millis()
#define OPL_ATTACH_TIMEOUT(_callback, _ms)  TIM4_timeout_attach(_callback, _ms)

///////////////////////////////////  L I N  ////////////////////////////////////

#include "gpio.h"
#define LIN_PORT PD
#define LIN_PIN  4

/* Configure the LIN transceiver control pin (CS) as a pushpull output */
#define OPL_LIN_INIT() do { \
    gpio_set_output(LIN_PORT, LIN_PIN); \
    gpio_set_pushpull(LIN_PORT, LIN_PIN); \
} while(0)

/* Enable write mode on the LIN transceiver, set CS pin to high */
#define OPL_LIN_ENABLE_TX()     gpio_write_high(LIN_PORT, LIN_PIN)

/* Disable write mode on the LIN transceiver, set CS pin to low */
#define OPL_LIN_DISABLE_TX()    gpio_write_low(LIN_PORT, LIN_PIN)


//////////////////////////////////  U A R T  ///////////////////////////////////

#include "uart.h"

#define UART_TX_PORT PD
#define UART_TX_PIN 5
#define UART_RX_PORT PD
#define UART_RX_PIN 6

/* 9-bit UART @ 9600 bauds */
#define OPL_UART_INIT(_addr, _callback)     uart_init(_addr, _callback)
#define OPL_UART_IS_ADDR()                  uart_is_addr()
#define OPL_UART_IS_BUSY()                  uart_is_busy()
#define OPL_UART_CLEAR_BUSY()               uart_clear_busy_flag()
#define OPL_UART_SET_ADDR(_addr)            uart_set_addr(_addr)
#define OPL_UART_MUTE()                     uart_mute()
#define OPL_UART_FLUSH_RX()                 uart_flush_rx_buffer()
#define OPL_UART_PEEK_BYTE()                uart_peek_byte()
#define OPL_UART_READ_BYTE()                uart_read_byte()
#define OPL_UART_WRITE_BYTE(_byte)          uart_write(_byte)
#define OPL_UART_WRITE_ADDR(_addr)          uart_write_addr(_addr)
#define OPL_UART_WRITE_BREAK()              uart_write_break()
#define OPL_UART_ENABLE_RX()                UART_ENABLE_RX()
#define OPL_UART_DISABLE_RX()               UART_DISABLE_RX()
#define OPL_READ_RX_PIN()                   gpio_read(UART_RX_PORT, UART_RX_PIN)


///////////////////////////////  S T O R A G E  ////////////////////////////////

#ifdef SLAVE

#include "eeprom.h"

/* EEPROM addresses */
#define MODE_ADDR    0x4000
#define SEED_ADDR    0x4001 // 4 bytes reserved for the seed
#define UID_ADDR     0x4005 // 12 bytes reserved for the UID

/* Get the operation mode: 0 = NC, 1 = No UID, 2 = Has UID */
#define OPL_LOAD_MODE() eeprom_read_uint8(MODE_ADDR)

/* Get a uint32 seed for the srand func, try to keep it random */
#define OPL_LOAD_SEED() eeprom_read_uint32(SEED_ADDR)

/* Get the unique ID */
#define OPL_LOAD_UID(_uid_ptr) eeprom_read_string(UID_ADDR, _uid_ptr, UID_SIZE)

#endif /* SLAVE */

#ifdef __cplusplus
}
#endif

#endif /* OPL_ADAPTERS_H */
