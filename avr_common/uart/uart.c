#include "uart.h"
#include <avr/io.h>
#include <stdio.h>

// ---- Configurazione UART ----
#define BAUD 19200
#define MYUBRR F_CPU/16/BAUD-1

// ---- Stream per printf ----
static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

// ---- Inizializzazione ----
void usart_init(uint16_t ubrr) {
    // Imposta baud rate
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    // Frame: 8 bit, 1 stop, no parity
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);

    // Abilita TX e RX
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}

// ---- Funzioni di base ----
void usart_putchar(char data) {
    // attesa buffer libero
    while (!(UCSR0A & _BV(UDRE0)));
    UDR0 = data;
}

char usart_getchar(void) {
    // attesa ricezione
    while (!(UCSR0A & _BV(RXC0)));
    return UDR0;
}

unsigned char usart_kbhit(void) {
    // ritorna 1 se un carattere è disponibile
    return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

void usart_pstr(char *s) {
    // stampa stringa
    while (*s) {
        usart_putchar(*s++);
    }
}

// ---- Integrazione con printf ----
int usart_putchar_printf(char var, FILE *stream) {
    // traduce \n in \r\n
    if (var == '\n') {
        usart_putchar('\r');
    }
    usart_putchar(var);
    return 0;
}

void printf_init(void) {
    // collega printf a UART
    stdout = &mystdout;
    // inizializza UART
    usart_init(MYUBRR);
}
