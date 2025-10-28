#pragma once

#include <stdint.h>  

/* ------------------------------------------------------------
   Indirizzo I2C del modulo OLED 
------------------------------------------------------------ */
#define OLED_ADDR 0x3C

/* ------------------------------------------------------------
   Inizializzazione e controllo base
------------------------------------------------------------ */
void OLED_init(void);
void OLED_clear(void);

/* ------------------------------------------------------------
   Stampa testo su riga (0–7)
------------------------------------------------------------ */
void OLED_print_line(uint8_t line, const char *text);

/* ------------------------------------------------------------
   Visualizzazione valore sensori
------------------------------------------------------------ */
void OLED_show_sensor(const char* temp, const char* press, const char* hum);
void OLED_show_sensors(const char *temp, const char *press, const char *hum);




