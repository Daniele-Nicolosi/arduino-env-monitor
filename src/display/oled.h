#pragma once
#include <stdint.h>

void oled_init(void);
void oled_clear(void);
void oled_print_line(uint8_t line, const char *text);

// Nuova funzione per mostrare i valori dei sensori
void oled_show_sensors(const char *temp, const char *press, const char *hum);

