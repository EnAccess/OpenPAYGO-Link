/*
 * Filename:    pwm.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  03/11/2020
 * Description: Based on https://blog.junix.in/2018/01/24/pwm-with-stm8s/.
 */

#include <stdint.h>
#include "gpio.h"
#include "pwm.h"

void config_pwm_pin() {
    gpio_set_output(PD, 3);
    gpio_set_pushpull(PD, 3);

    TIM2_PSCR = 0x00; // Prescaler = 1

    // Fill 16 bit timer2_arr to two 8 bit registers.
    // MSB register to be filled first.
    // PWM with 2kHz frequency
    TIM2_ARRH = 0x03; // 999 >> 8;
    TIM2_ARRL = 0xE7; // 999 & 0x00FF;

    // Fill 16 bit timer2_ccr1 to two 8 bit registers.
    // MSB register to be filled first.
    TIM2_CCR2H = 0x00; // 0 >> 8;
    TIM2_CCR2L = 0x00; // 0 & 0x00FF;

    TIM2_CCER1 |= (1 << 4); // Enable channel 1 output, active high

    // PWM mode 1.
    // Set output compare mode as 6 (0b110)
    // So channel 1 will be active while counter
    // is lower than compare value.
    // Enable preload, bit 3
    TIM2_CCMR2 |= 0b01101000;

    TIM2_CR1 |= (1<<TIM2_CR1_CEN); // Enable counter
}

void set_pwm_duty_cycle(uint16_t value) {
    if(value > 999) value = 999;
    TIM2_CCR2H = value >> 8;
    TIM2_CCR2L = value & 0x00FF;
}
