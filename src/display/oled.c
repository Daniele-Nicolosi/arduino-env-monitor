#include "oled.h"
#include "font/font.h"
#include "../../avr_common/i2c/i2c.h"
#include <util/delay.h>

// --- Costanti ---
#define OLED_ADDR 0x3C   // indirizzo tipico SH1106

// --- Funzioni interne ---
static void oled_command(uint8_t cmd) {
    i2c_write_reg(OLED_ADDR, 0x00, cmd);   // 0x00 = modalità comando
}

static void oled_data(uint8_t data) {
    i2c_write_reg(OLED_ADDR, 0x40, data);  // 0x40 = modalità dati
}

// --- Init ---
void oled_init(void) {
    _delay_ms(100);

    oled_command(0xAE); // display off
    oled_command(0xD5); oled_command(0x80);
    oled_command(0xA8); oled_command(0x3F);
    oled_command(0xD3); oled_command(0x00);
    oled_command(0x40);
    oled_command(0xAD); oled_command(0x8B); // charge pump
    oled_command(0xA1); 
    oled_command(0xC8);
    oled_command(0xDA); oled_command(0x12);
    oled_command(0x81); oled_command(0x80);
    oled_command(0xD9); oled_command(0x22);
    oled_command(0xDB); oled_command(0x35);
    oled_command(0xA4);
    oled_command(0xA6);
    oled_command(0xAF); // display ON

    oled_clear();
}

// --- Clear ---
void oled_clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        oled_command(0xB0 + page); // pagina (0–7)
        oled_command(0x02);        // colonna low (SH1106 usa offset 2)
        oled_command(0x10);        // colonna high
        for (uint8_t col = 0; col < 128; col++) {
            oled_data(0x00);
        }
    }
}

// --- Print line ---
void oled_print_line(uint8_t line, const char *text) {
    if (line > 7) return;

    oled_command(0xB0 + line); // seleziona pagina
    oled_command(0x02);        // colonna low
    oled_command(0x10);        // colonna high

    while (*text) {
        char c = *text++;
        if (c < 32 || c > 126) c = '?'; // caratteri non supportati
        const uint8_t *glyph = &oled_font5x7[(c - 32) * 5];
        for (uint8_t i = 0; i < 5; i++) {
            oled_data(glyph[i]);
        }
        oled_data(0x00); // spazio tra caratteri
    }
}
