/*
 * Filename:    gpio.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  10/01/2020
 * Description: GPIO macros.
 */

#ifndef GPIO_H
#define GPIO_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "stm8s.h"

#define gpio_set_input(_port, _pin)      (_port->DDR &= ~(1 << _pin))
#define gpio_set_output(_port, _pin)     (_port->DDR |=  (1 << _pin))

#define gpio_set_opendrain(_port, _pin)  (_port->CR1 &= ~(1 << _pin))
#define gpio_set_pushpull(_port, _pin)   (_port->CR1 |=  (1 << _pin))

#define gpio_write_low(_port, _pin)      (_port->ODR &= ~(1 << _pin))
#define gpio_write_high(_port, _pin)     (_port->ODR |=  (1 << _pin))

#define gpio_toggle(_port, _pin)         (_port->ODR ^= (1 << _pin))

#define gpio_read(_port, _pin)           ((_port->IDR & (1 << _pin)) >> _pin)


#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
