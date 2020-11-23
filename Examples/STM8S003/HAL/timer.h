/*
 * Filename:    timer.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  10/01/2020
 * Description: Based on a file by Georg Icking-Konert
 *              (https://github.com/gicking).
 */

#ifndef TIMER_H
#define TIMER_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm8s.h"

void timer_isr() __interrupt(TIM4_ISR);

void TIM4_init();

uint32_t millis();

void TIM4_timeout_attach(void (*callback)(void), uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_H */
