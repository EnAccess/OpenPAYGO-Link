/*
 * Filename:    timer.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  10/01/2020
 * Description: Based on a file by Georg Icking-Konert
 *              (https://github.com/gicking).
 */

#include <stdint.h>
#include <stddef.h>
#include "stm8s.h"
#include "timer.h"

volatile uint32_t g_millis;

void timer_isr() __interrupt(TIM4_ISR) { // Every 1ms
    static uint16_t elapsed_millis = 0;
    g_millis++;

    TIM4_SR &= ~(1 << TIM4_SR_UIF); // Clear TIM4 flags
}

void TIM4_init() {
  g_millis = 0;
  TIM4_CR1 &= ~(1 << TIM4_CR1_CEN); // Disable TIM4
  TIM4_CNTR = 0x00; // Clear counter
  TIM4_CR1 |= (1 << TIM4_CR1_ARPE); // Auto-reload value buffered
  TIM4_EGR = 0x00; // Clear pending events

  TIM4_PSCR = 0b00000100; // Set clock to 2Mhz/2^4 = 125kHz -> 8us period
  TIM4_ARR = 125; // set autoreload value for 1ms (=125*4us)

  TIM4_IER |= (1 << TIM4_IER_UIE); // Enable Update Interrupt
  TIM4_CR1 |= (1 << TIM4_CR1_CEN); // Enable TIM4
}

uint32_t millis() {
    return g_millis;
}
