#pragma once
#include <stdint.h>

// --- Funzioni pubbliche ---

// Inizializza il display SH1106
void oled_init(void);

// Pulisce tutto lo schermo
void oled_clear(void);

// Stampa una stringa nella riga indicata (0–7)
void oled_print_line(uint8_t line, const char *text);
