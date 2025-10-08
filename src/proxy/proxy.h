#pragma once
#include <stdint.h>

/* ------------------------------------------------------------
   Tipi di configurazione proxy
------------------------------------------------------------ */
typedef enum {
    UNIT_C = 0,  // Celsius
    UNIT_K = 1,  // Kelvin
    UNIT_F = 2   // Fahrenheit
} temp_unit_t;

typedef enum {
    UNIT_PA  = 0,  // Pascal
    UNIT_BAR = 1   // Bar
} press_unit_t;

/* ------------------------------------------------------------
   API del proxy
------------------------------------------------------------ */
void proxy_init(void);  // Inizializza UART, I2C, sensore, OLED e pulsanti
void proxy_run(void);   // Avvia il menù interattivo gestito dai pulsanti













