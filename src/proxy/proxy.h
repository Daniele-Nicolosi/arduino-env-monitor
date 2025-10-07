#pragma once
#include <stdint.h>

typedef enum {
    UNIT_C = 0,  // Celsius
    UNIT_K = 1,  // Kelvin
    UNIT_F = 2   // Fahrenheit
} temp_unit_t;

typedef enum {
    UNIT_PA = 0,  // Pascal 
    UNIT_BAR = 1  // Bar
} press_unit_t;

// Inizializza UART, I2C, sensore, OLED e pulsanti
void proxy_init(void);

// Avvia il menu interattivo gestito tramite pulsanti
void proxy_run(void);












