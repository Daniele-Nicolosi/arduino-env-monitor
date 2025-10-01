#include "../avr_common/uart.h"
#include "../avr_common/i2c.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

int main(void) {
    printf_init();
    i2c_init();
    sei();

    printf("I2C Scanner avviato\n");

    for (uint8_t addr = 1; addr < 127; addr++) {
        uint8_t status = i2c_start(addr, I2C_WRITE);
        if (status == 0x18 || status == 0x40) {
            printf("Trovato dispositivo a 0x%02X\n", addr);
        }
        i2c_stop();
        _delay_ms(5);
    }

    printf("Scanner completato\n");

    while (1);
}

