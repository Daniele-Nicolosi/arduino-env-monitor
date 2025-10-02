#include "../avr_common/uart/uart.h"
#include "../avr_common/i2c/i2c.h"
#include "sensors/bme280.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>  // dtostrf

int main(void) {
    printf_init();   // UART pronta per printf
    i2c_init();      // I2C @100kHz
    sei();

    bme280_init();   // carica calibrazioni + config sensore

    char tbuf[16], pbuf[20], hbuf[16];

    while (1) {
        // IMPORTANTE: leggi la temperatura per prima (aggiorna t_fine)
        float t = bme280_read_temperature();   // °C
        float p = bme280_read_pressure();      // hPa
        float h = bme280_read_humidity();      // %RH

        // Converti float in stringhe (così non servono -lprintf_flt)
        dtostrf(t, 6, 2, tbuf);
        dtostrf(p, 7, 2, pbuf);   // hPa: tipicamente ~1000.xx
        dtostrf(h, 6, 2, hbuf);

        printf("Temperatura = %s C, Pressione = %s hPa, Umidita = %s %%\n",
               tbuf, pbuf, hbuf);

        _delay_ms(2000);
    }
}




