#pragma once
#include <avr/io.h>
#include <stdint.h>

// Definizione pin (puoi cambiare)
#define BTN1_PIN   PD2   // Digital 2
#define BTN2_PIN   PD3   // Digital 3

// Macro per leggere stato
#define BTN1_PRESSED()  (!(PIND & (1 << BTN1_PIN)))
#define BTN2_PRESSED()  (!(PIND & (1 << BTN2_PIN)))

// Inizializzazione
void buttons_init(void);
