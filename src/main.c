#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "proxy/proxy.h"
#include "display/oled.h"
#include "sensors/bme280.h"
#include "../avr_common/uart/uart.h"
#include "../avr_common/i2c/i2c.h"

int main(void) {
    // Inizializzazione proxy (UART + I2C + BME280 + OLED)
    proxy_init();

    // Messaggio iniziale su OLED
    oled_clear();
    oled_print_line(0, "       WELCOME!");
    oled_print_line(4, "Use the PC to send");
    oled_print_line(5, "commands");

    // Avvia il loop proxy: riceve comandi da Cutecom e aggiorna display
    proxy_run();

    // Se esci con "q" o "exit", mostra un messaggio di chiusura
    oled_clear();
    oled_print_line(3, "      GOODBYE! :)");
    _delay_ms(3000);
    oled_clear();
    while (1) {
        // fine programma
    }

    return 0;
}



































