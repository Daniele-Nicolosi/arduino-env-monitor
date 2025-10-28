#include "buttons.h"

/* ------------------------------------------------------------
   Inizializzazione dei pin dei pulsanti
   - Usa INPUT_PULLUP → logica inversa (premuto = 0)
------------------------------------------------------------ */
void BUTTONS_init(void) {
    DDRD &= ~((1 << BTN_SELECT_PIN) | (1 << BTN_CONFIRM_PIN)); // input
    PORTD |= (1 << BTN_SELECT_PIN) | (1 << BTN_CONFIRM_PIN);   // pull-up attivi
}

/* ------------------------------------------------------------
   Lettura con debounce (semplice)
   Ritorna:
     1 = select
     2 = confirm
     0 = nessuno
------------------------------------------------------------ */
uint8_t BUTTONS_read(void) {
    if (!(PIND & (1 << BTN_SELECT_PIN))) {
        _delay_ms(30); // debounce
        if (!(PIND & (1 << BTN_SELECT_PIN))) return 1;
    }

    if (!(PIND & (1 << BTN_CONFIRM_PIN))) {
        _delay_ms(30);
        if (!(PIND & (1 << BTN_CONFIRM_PIN))) return 2;
    }

    return 0;
}


