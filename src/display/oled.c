#include "oled.h"
#include "font/font.h"
#include "../../avr_common/i2c/i2c.h"
#include <util/delay.h>
#include <string.h>

/* ------------------------------------------------------------
   Funzioni interne (comandi e dati I2C)
------------------------------------------------------------ */
static void oled_command(uint8_t cmd) {
    i2c_write_reg(OLED_ADDR, 0x00, cmd); // invia comando
}

static void oled_data(uint8_t data) {
    i2c_write_reg(OLED_ADDR, 0x40, data); // invia dato
}

/* ------------------------------------------------------------
   oled_init()
   Inizializza il display SH1106
------------------------------------------------------------ */
void oled_init(void) {
    _delay_ms(100);

    oled_command(0xAE); // display off
    oled_command(0xD5); oled_command(0x80);
    oled_command(0xA8); oled_command(0x3F);
    oled_command(0xD3); oled_command(0x00);
    oled_command(0x40);
    oled_command(0xAD); oled_command(0x8B); 
    oled_command(0xA1); 
    oled_command(0xC8);
    oled_command(0xDA); oled_command(0x12);
    oled_command(0x81); oled_command(0x80);
    oled_command(0xD9); oled_command(0x22);
    oled_command(0xDB); oled_command(0x35);
    oled_command(0xA4);
    oled_command(0xA6);
    oled_command(0xAF); // display on

    oled_clear();
}

/* ------------------------------------------------------------
   oled_clear()
   Pulisce lo schermo (8 pagine × 128 colonne)
------------------------------------------------------------ */
void oled_clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        oled_command(0xB0 + page);
        oled_command(0x02);
        oled_command(0x10);
        for (uint8_t col = 0; col < 128; col++) {
            oled_data(0x00);
        }
    }
}

/* ------------------------------------------------------------
   oled_print_line()
   Scrive testo su una riga (pagina 0–7)
------------------------------------------------------------ */
void oled_print_line(uint8_t line, const char *text) {
    if (line > 7) return;

    oled_command(0xB0 + line);
    oled_command(0x02);
    oled_command(0x10);

    while (*text) {
        char c = *text++;
        if (c < 32 || c > 126) c = '?';
        const uint8_t *glyph = &oled_font5x7[(c - 32) * 5];
        for (uint8_t i = 0; i < 5; i++) oled_data(glyph[i]);
        oled_data(0x00);
    }
}

/* ------------------------------------------------------------
   oled_show_sensor()
   Mostra un solo valore (temp, press o hum)
------------------------------------------------------------ */
void oled_show_sensor(const char* temp, const char* press, const char* hum) {
    oled_clear();
    if (temp)  { oled_print_line(3, temp);  return; }
    if (press) { oled_print_line(3, press); return; }
    if (hum)   { oled_print_line(3, hum);   return; }
}

/* ------------------------------------------------------------
   oled_show_sensors()
   Mostra tre valori su linee 1, 3 e 5
------------------------------------------------------------ */
void oled_show_sensors(const char *temp, const char *press, const char *hum) {
    oled_clear();
    oled_print_line(1, temp); 
    oled_print_line(3, press);
    oled_print_line(5, hum);
}




