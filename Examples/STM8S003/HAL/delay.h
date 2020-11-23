/*
 * Filename:    delay.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  10/01/2020
 * Description: Based on a file by Lujji (https://github.com/lujji).
 */

#ifndef DELAY_H
#define DELAY_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm8s.h"

#ifndef F_CPU
#warning "F_CPU not defined, using 2MHz by default"
#define F_CPU 2000000UL
#endif

inline void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ((F_CPU / 18 / 1000UL) * ms); i++) {
        nop();
    }
}

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H */
