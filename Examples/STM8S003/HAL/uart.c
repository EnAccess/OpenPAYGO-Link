/*
 * Filename:    uart.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  21/01/2020
 * Description: UART software driver with Rx FIFO buffer and different working
 *              modes. Based on a file by Lujji (https://github.com/lujji).
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm8s.h"
#include "uart.h"

struct {
    bool busy : 1;
    bool mute : 1;
    bool is_addr : 1;
    uint8_t default_addr : 4;
    uint8_t addr : 4;
} uart;

typedef struct {
    uint8_t data_buffer[UART_BUFFER_SIZE];
    uint8_t iFirst;
    uint8_t iLast;
    void (*callback)(uint8_t);
} swFIFO_t;

swFIFO_t rx;

void uart_init(uint8_t default_addr, void(*rx_callback)(uint8_t)) {

    UART1_CR2 &= ~((1 << UART1_CR2_TEN) | (1 << UART1_CR2_REN)); // RX & TX off

    /* Set the baudrate to 9600 bauds. BRRx values depend on clock speed */
    UART1_BRR2 = 0x00; // 0x08 <- 19200
    UART1_BRR1 = 0x0D; // 0x06 <- 19200

    // Set uart to 9 bit mode
    UART1_CR1 |= (1 << 4);

    uart.mute = true;
    uart.default_addr = default_addr;
    uart.addr = default_addr;
    uart.busy = false;

    uart_flush_rx_buffer(); // This enables the UART interrupts

    if(rx_callback) rx.callback = rx_callback;
    else rx.callback = NULL;

    UART1_CR2 |= (1<<5); // (UART1_CR2_RIEN) Enable RX interrupt
    UART1_CR2 |= (1 << UART1_CR2_TEN) | (1 << UART1_CR2_REN); // Enable RX & TX

    // Clean the data register
    (void)UART1_SR;
    (void)UART1_DR;
}

void uart_set_addr(uint8_t addr) {
    if(addr <= 0x0F) uart.addr = addr; // The default addr remains the same
}

void uart_mute() {
    uart.mute = true;
}

void uart_isr() __interrupt(UART1_RXC_ISR) {
    uart.busy = true;
    if(reg_read_bit(UART1_SR, UART1_SR_FE) == 0) { // No framing error
        uint8_t byte = UART1_DR; // Read data register AFTER status register
        uart.is_addr = reg_read_bit(UART1_CR1, UART1_CR1_R8);

        if(uart.is_addr) { // 9th bit is set
            uint8_t addr = byte & 0x0F;
            if(addr == uart.addr || addr == uart.default_addr) {
                uart.mute = false; // Wake up, we received an address byte
                rx.iFirst = rx.iLast; // Flush the FIFO
            }
            else
                uart.mute = true;
        }

        uint8_t i = (rx.iLast + 1) % UART_BUFFER_SIZE;
        if(uart.mute == false && i != rx.iFirst) {
            rx.data_buffer[rx.iLast] = byte; // Push the new byte to the FIFO
            rx.iLast = i;
            if(rx.callback) rx.callback(byte);
        }
    }
    else {
        (void)UART1_DR; // Dummy read to clear the flags
    }
}

bool uart_is_addr() {
    return uart.is_addr;
}

bool uart_is_busy() {
    return uart.busy;
}

void uart_clear_busy_flag() {
    uart.busy = false;
}

uint8_t uart_peek_byte() {
    return rx.data_buffer[rx.iLast-1]; // Because iLast is empty
}

uint8_t uart_read_byte() {
    uint8_t byte = 0;

    if(rx.iLast != rx.iFirst){
        UART1_CR2 &= ~(1<<5); // Disable RX interrupt

        byte = rx.data_buffer[rx.iFirst];
        rx.iFirst = (rx.iFirst + 1) % UART_BUFFER_SIZE;

        UART1_CR2 |= (1<<5); // Enable RX interrupt
    }

    return byte;
}

void uart_write(uint8_t data) {
    UART1_DR = data;
    while (!(UART1_SR & (1 << UART1_SR_TC)));
}

void uart_write_addr(uint8_t addr) {
    UART1_CR1 |= (1<<6); // Set address bit
    uart_write(addr);
    UART1_CR1 &= ~(1<<6); // Reset address bit
}

void uart_write_break() {
    UART1_CR2 |= (1 << UART1_CR2_SBK);
}

void uart_flush_rx_buffer() {
    // Virtually clear the FIFO buffer setting the tail to the head.
    UART_DISABLE_ISR();
    rx.iFirst = rx.iLast;
    UART_ENABLE_ISR();
}
