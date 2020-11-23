/*
 * Filename:    eeprom.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  10/01/2020
 * Description: EEPROM interface. Based on a file by Lujji
 *              (https://github.com/lujji).
 */

#ifndef EEPROM_H
#define EEPROM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "stm8s.h"

#define EEPROM_START_ADDR   0x4000
#define EEPROM_END_ADDR     0x407F

uint8_t eeprom_read_uint8(uint16_t addr);

void eeprom_write_uint8(uint16_t addr, uint8_t data);

uint32_t eeprom_read_uint32(uint16_t addr);

void eeprom_write_uint32(uint16_t addr, uint32_t data);

void eeprom_read_string(uint16_t addr, uint8_t *buf, uint8_t len);

void eeprom_write_string(uint16_t addr, const uint8_t *buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* EEPROM_H */
