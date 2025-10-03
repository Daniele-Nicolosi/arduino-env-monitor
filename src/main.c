#include <avr/io.h>
#include <util/delay.h>
#include "../avr_common/i2c/i2c.h"
#include "display/oled.h"

int main(void) {
    // --- Inizializzazioni ---
    i2c_init();    // inizializza I2C
    oled_init();   // inizializza display

    // --- Test di scrittura ---
    oled_print_line(0, "Hello SH1106!");
    oled_print_line(1, "Test display I2C");
    oled_print_line(2, "Temperatura: 22 C");
    oled_print_line(3, "Pressione: 1009 hPa");
    oled_print_line(4, "Umidita: 48%");

    while (1) {
        _delay_ms(1000);
        // loop infinito, il testo resta sul display
    }
}
















