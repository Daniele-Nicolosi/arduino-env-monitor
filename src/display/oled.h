#pragma once

#include <stdint.h>  

/* ------------------------------------------------------------
   Indirizzo I2C del modulo OLED 
------------------------------------------------------------ */
#define OLED_ADDR 0x3C

/* ------------------------------------------------------------
   Inizializzazione e controllo base
------------------------------------------------------------ */
void oled_init(void);
void oled_clear(void);

/* ------------------------------------------------------------
   Stampa testo su riga (0–7)
------------------------------------------------------------ */
void oled_print_line(uint8_t line, const char *text);

/* ------------------------------------------------------------
   Visualizzazione valore sensori
------------------------------------------------------------ */
void oled_show_sensor(const char* temp, const char* press, const char* hum);
void oled_show_sensors(const char *temp, const char *press, const char *hum);



