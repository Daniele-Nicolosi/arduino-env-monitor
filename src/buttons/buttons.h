#pragma once

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

/* ------------------------------------------------------------
   Definizione pin dei pulsanti
------------------------------------------------------------ */
#define BTN_SELECT_PIN PD2   // Pulsante per scorrere i parametri
#define BTN_CONFIRM_PIN PD3  // Pulsante per confermare la scelta

/* ------------------------------------------------------------
   Inizializzazione dei pulsanti
------------------------------------------------------------ */
void BUTTONS_init(void);

/* ------------------------------------------------------------
   Legge lo stato dei pulsanti 
   Ritorna:
     1 se BTN_SELECT premuto
     2 se BTN_CONFIRM premuto
     0 se nessuno
------------------------------------------------------------ */
uint8_t BUTTONS_read(void);

