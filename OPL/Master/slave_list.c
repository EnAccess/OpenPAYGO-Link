/*
 * Filename:    slave_list.h
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/03/2020
 * Description: Slave management functions.
 */

#include <stdint.h>
#include <string.h>
#include "slave_list.h"
#include "slave_list_private.h"

slave_t slaves[MAX_SLAVES];

void slave_list_init() {
    for(uint8_t i = 0; i < MAX_SLAVES; i++) {
        slave_clear_slot(i);
    }
}

uint8_t slave_list_available() {
    for(uint8_t i = 0; i < MAX_SLAVES; i++)
        if(slaves[i].addr == 0x00)
            return i + 1; // Valid addresses start from 1
    return 0;
}

bool slave_list_add(uint8_t new_addr, uint8_t *uid, uint8_t len) {
    uint8_t index = new_addr - 1;
    if(slaves[index].addr == 0x00) { // Convert from addr to index
        slaves[index].addr = new_addr;
        if(uid != NULL) memcpy(slaves[index].uid, uid, len);
        return true;
    }
    else{
        return false;
    }
}

void slave_clear_slot(uint8_t index) {
    slaves[index].addr = 0x00;
    memset(slaves[index].uid, 0, UID_SIZE);
    slaves[index].ping_count = 0xFF;
    slaves[index].ping_error = 0;
}

void slave_ping_error(uint8_t addr) {
    if(++(slaves[--addr].ping_error) == MAX_PING_ERROR) // Convert to index
        slave_clear_slot(addr); // Pass the address
}

void slave_set_ping_period(uint8_t addr, uint8_t ticks) {
    if(ticks) {
        slaves[--addr].ping_count = ticks; // Convert from addr to index
        slaves[addr].ping_error = 0;
    }
}

static bool ping_flag = false;

void slave_list_ping_tick(){
    for(uint8_t i = 0; i < MAX_SLAVES; i++)
        if( (slaves[i].addr != 0x00) && (slaves[i].ping_count > 0) )
            if(--slaves[i].ping_count == 0)
                ping_flag = true; // At least one slave needs to be pinged
}

/* This function implements a circular structure to verify if a slave needs to
 * be pinged. There are two for loops, checking from the last index up to the
 * max and from 0 up to the last index. The reason for this is to make sure
 * all the nodes are visited while keeping track of what node was pinged last.
 */
uint8_t next_slave_ping() {
    static uint8_t last = 0;

    if(ping_flag == false) return 0; // Early return to skip the loops

    uint8_t this = last;

    for( ; last < MAX_SLAVES; last++)
        if(slaves[last].ping_count == 0)
            return ++last; // Increase for next call & to convert to address

    for(last = 0; last < this; last++)
        if(slaves[last].ping_count == 0)
            return ++last; // Increase for next call & to convert to address

    ping_flag = false; // If we are here it is because no slave needs ping
    return 0;
}

void get_slave_list(opl_slave_list_t *ptr) {
    uint8_t count = 0;
    for(uint8_t i = 0; i < MAX_SLAVES; i++) {
        if(slaves[i].addr != 0) {
            memcpy(ptr->uids[i], slaves[i].uid, strlen(slaves[i].uid));
            count++;
        }
    }
    ptr->n_slaves = count;

    for(uint8_t i = count; i < MAX_SLAVES; i++){
        ptr->uids[i][0] = '\0'; // Set first value of each uid slot to 0
    }
}

uint8_t _map_uid_to_addr(uint8_t *uid) {
    for(uint8_t i = 0; i < MAX_SLAVES; i++)
        if(memcmp(uid, slaves[i].uid, strlen(slaves[i].uid)) == 0)
            return slaves[i].addr;
    return 0;
}
