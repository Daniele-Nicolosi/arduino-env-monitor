#include "buttons.h"

/* ------------------------------------------------------------
   buttons_init()
   Configura i pin dei pulsanti come input con pull-up attivo.
------------------------------------------------------------ */
void buttons_init(void) {
    // Imposta D2 e D3 come input
    DDRD &= ~((1 << BTN1_PIN) | (1 << BTN2_PIN));

    // Abilita pull-up interni
    PORTD |= (1 << BTN1_PIN) | (1 << BTN2_PIN);
}
