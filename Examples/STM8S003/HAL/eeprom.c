/*
 * Filename:    eeprom.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  10/01/2020
 * Description: EEPROM interface. Based on a file by Lujji
 *              (https://github.com/lujji).
 */

#include "eeprom.h"

static void eeprom_unlock() {
    FLASH_DUKR = FLASH_DUKR_KEY1;
    FLASH_DUKR = FLASH_DUKR_KEY2;
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_DUL)));
}

static void eeprom_lock() {
    FLASH_IAPSR &= ~(1 << FLASH_IAPSR_DUL);
}

static void eeprom_wait_busy() {
    while (!(FLASH_IAPSR & (1 << FLASH_IAPSR_EOP)));
}

static uint8_t eeprom_read_byte(uint16_t addr) {
    return _MEM_(addr);
}

static void eeprom_write_byte(uint16_t addr, uint8_t data) {
    _MEM_(addr) = data;
}

uint8_t eeprom_read_uint8(uint16_t addr) {
    return eeprom_read_byte(addr);
}

void eeprom_write_uint8(uint16_t addr, uint8_t data) {
    eeprom_unlock();
    eeprom_write_byte(addr, data);
    eeprom_lock();
}

uint32_t eeprom_read_uint32(uint16_t addr) {
    uint32_t result;
    result = eeprom_read_byte(addr);
    result <<= 8;
    result += eeprom_read_byte(addr + 1);
    result <<= 8;
    result += eeprom_read_byte(addr + 2);
    result <<= 8;
    result += eeprom_read_byte(addr + 3);
    return result;
}

void eeprom_write_uint32(uint16_t addr, uint32_t data) {
    eeprom_unlock();
    eeprom_write_byte(addr, data >> 24 & 0x000000FF);
    eeprom_write_byte(addr + 1, data >> 16 & 0x000000FF);
    eeprom_write_byte(addr + 2, data >> 8 & 0x000000FF);
    eeprom_write_byte(addr + 3, data & 0x000000FF);
    eeprom_lock();
}

void eeprom_read_string(uint16_t addr, uint8_t *buf, uint8_t len) {
    for(uint16_t i = 0; i < len; i++){
        buf[i] = eeprom_read_byte(addr);
        addr++;
    }
}

void eeprom_write_string(uint16_t addr, const uint8_t *buf, uint8_t len) {
    eeprom_unlock();
    for(uint8_t i = 0; i < len; i++, addr++) {
        eeprom_write_byte(addr, buf[i]);
    }
    eeprom_lock();
}
