/*
 * Filename:    main.c
 * Project:     OpenPAYGO Link
 * Author:      Daniel Nedosseikine
 * Company:     Solaris Offgrid
 * Created on:  12/11/2020
 * Description: Basic single node example. Master code.
 */

#include "string.h"
#include "timer.h"
#include "gpio.h"
#include "oplink_master.h"

uint8_t buffer[128];
char message[] = "OpenPAYGO";
char reply[] = "Link";

void main() {

    disable_interrupts(); // Probably disabled by default

    /******* MCU specific stuff ********/
    TIM4_init();
    gpio_set_output(PB, 5);
    gpio_set_pushpull(PB, 5);
    gpio_write_high(PB, 5); // The LED has inverted logic
    /***********************************/

    opl_init();

    enable_interrupts();

    size_t sz;
    opl_slave_list_t list;
    uint32_t ms = 0;
    uint32_t message_ms = 0;
    uint32_t led_ms = 0;

    while(1) {

        ms = millis();

        // Send OpenPAYGO every 5 seconds
        if(ms - message_ms > 5000) {
            message_ms = ms;
            get_slave_list(&list);
            if(list.uids[0][0] != 0) {
                opl_push_request(list.uids[0], message, strlen(message));
            }
        }

        // Turn off the LED after 0.5 seconds
        if( (gpio_read(PB, 5) == 0) && (ms - led_ms > 500) ) {
            gpio_write_high(PB, 5);
        }

        // Handle incoming messages
        if((sz = opl_parse()) > 0){
            if(opl_read(buffer, sz)){
                if(memcmp(buffer, reply, sz) == 0) {
                    gpio_write_low(PB, 5);
                    led_ms = ms;
                }
            }
        }

        // OpenPAYGO Link internal routines
        opl_keep_alive();
    }
}
