/*
 * Filename:    pwm.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  03/11/2020
 * Description: Based on https://blog.junix.in/2018/01/24/pwm-with-stm8s/.
 */

#ifndef PWM_H
#define PWM_H 1

#ifdef __cplusplus
extern "C" {
#endif

void config_pwm_pin();

void set_pwm_duty_cycle(uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* PWM_H */
