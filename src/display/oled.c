#include <util/delay.h>
#include <string.h>

#include "../../avr_common/i2c/i2c.h"
#include "font/font.h"
#include "oled.h"

/* ------------------------------------------------------------
   Funzioni interne (comandi e dati I2C)
------------------------------------------------------------ */
static void OLED_command(uint8_t cmd) {
    I2C_write_reg(OLED_ADDR, 0x00, cmd); // invia comando
}

static void OLED_data(uint8_t data) {
    I2C_write_reg(OLED_ADDR, 0x40, data); // invia dato
}

/* ------------------------------------------------------------
   OLED_init()
   Inizializza il display SH1106
   Configurazione base 128x64, I2C
------------------------------------------------------------ */
void OLED_init(void) {
    _delay_ms(100);

    OLED_command(0xAE); // display off
    OLED_command(0xD5); OLED_command(0x80);
    OLED_command(0xA8); OLED_command(0x3F);
    OLED_command(0xD3); OLED_command(0x00);
    OLED_command(0x40);
    OLED_command(0xAD); OLED_command(0x8B); 
    OLED_command(0xA1); 
    OLED_command(0xC8);
    OLED_command(0xDA); OLED_command(0x12);
    OLED_command(0x81); OLED_command(0x80);
    OLED_command(0xD9); OLED_command(0x22);
    OLED_command(0xDB); OLED_command(0x35);
    OLED_command(0xA4);
    OLED_command(0xA6);
    OLED_command(0xAF); // display on

    OLED_clear();
}

/* ------------------------------------------------------------
   OLED_clear()
   Pulisce lo schermo (8 pagine × 128 colonne)
------------------------------------------------------------ */
void OLED_clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_command(0xB0 + page);
        OLED_command(0x02);
        OLED_command(0x10);
        for (uint8_t col = 0; col < 128; col++) {
            OLED_data(0x00);
        }
    }
}

/* ------------------------------------------------------------
   OLED_print_line()
   Scrive testo su una riga (pagina 0–7)
------------------------------------------------------------ */
void OLED_print_line(uint8_t line, const char *text) {
    if (line > 7) return;

    OLED_command(0xB0 + line);
    OLED_command(0x02);
    OLED_command(0x10);

    while (*text) {
        char c = *text++;
        if (c < 32 || c > 126) c = '?';
        const uint8_t *glyph = &OLED_font5x7[(c - 32) * 5];
        for (uint8_t i = 0; i < 5; i++) OLED_data(glyph[i]);
        OLED_data(0x00);
    }
}

/* ------------------------------------------------------------
   OLED_show_sensor()
   Mostra un solo valore (temp, press o hum)
------------------------------------------------------------ */
void OLED_show_sensor(const char* temp, const char* press, const char* hum) {
    OLED_clear();
    if (temp)  { OLED_print_line(3, temp);  return; }
    if (press) { OLED_print_line(3, press); return; }
    if (hum)   { OLED_print_line(3, hum);   return; }
}

/* ------------------------------------------------------------
   OLED_show_sensors()
   Mostra tre valori su linee 1, 3 e 5
------------------------------------------------------------ */
void OLED_show_sensors(const char *temp, const char *press, const char *hum) {
    OLED_clear();
    OLED_print_line(1, temp); 
    OLED_print_line(3, press);
    OLED_print_line(5, hum);
}





