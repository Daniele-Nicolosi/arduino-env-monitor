#include "../avr_common/uart/uart.h"
#include "../avr_common/i2c/i2c.h"
#include "sensors/bme280.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>  // dtostrf

int main(void) {
    printf_init();
    i2c_init();
    sei();

    bme280_init();

    char buf[16];

    while (1) {
        float t = bme280_read_temperature();
        dtostrf(t, 6, 2, buf); // converte float in stringa
        printf("Temperatura = %s C\n", buf);
        _delay_ms(2000);
    }
}



