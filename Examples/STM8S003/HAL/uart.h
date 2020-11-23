/*
 * Filename:    uart.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  21/01/2020
 * Description: UART software driver with Rx FIFO buffer and different working
 *              modes. Based on a file by Lujji (https://github.com/lujji).
 */

#ifndef UART_H
#define UART_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm8s.h"

#ifndef F_CPU
#warning "F_CPU not defined, using 2MHz by default"
#define F_CPU 2000000UL
#endif

#define UART_BUFFER_SIZE    128

#define UART_ENABLE_ISR()  (UART1_CR2 |= (1<<5))
#define UART_DISABLE_ISR() (UART1_CR2 &= ~(1<<5))

#define UART_ENABLE_RX()   (UART1_CR2 |= (1 << UART1_CR2_REN))
#define UART_DISABLE_RX()  (UART1_CR2 &= ~(1 << UART1_CR2_REN))

#define UART_ENABLE_TX()   (UART1_CR2 |= (1 << UART1_CR2_TEN))
#define UART_DISABLE_TX()  (UART1_CR2 &= ~(1 << UART1_CR2_TEN))

// flow-control: none. PD5 -> TX / PD6 -> RX
void uart_init(uint8_t default_addr, void(*rx_callback)(uint8_t));

void uart_set_addr(uint8_t addr); // Max length is 4 bits

void uart_mute();

void uart_isr() __interrupt(UART1_RXC_ISR);

bool uart_is_addr();

bool uart_is_busy();

void uart_clear_busy_flag();

uint8_t uart_peek_byte();

uint8_t uart_read_byte();

void uart_write(uint8_t data);

void uart_write_addr(uint8_t addr);

void uart_write_break();

void uart_flush_rx_buffer();

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
