#include "uart.h"

// --- Includes ---
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdint.h>

// --- Macros and Defines ---
#define BAUD 19200
#define MYUBRR F_CPU/16/BAUD-1

// --- Function Prototypes (local) ---
void usart_init(uint16_t ubrr);
char usart_getchar(void);
void usart_putchar(char data);
void usart_pstr(char *s);
unsigned char usart_kbhit(void);
int usart_putchar_printf(char var, FILE *stream);
void uart_puts(const char *s);
int uart_readline(char *buf, int maxlen);

/* Stream for printf */
static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

/* Send a NUL-terminated string via UART (public utility) */
void uart_puts(const char *s) {
    while (*s) {
        usart_putchar(*s++);
    }
}

/* Read a line from UART with echo and basic editing (backspace).
   Returns number of chars read (without terminating NUL). */
int uart_readline(char *buf, int maxlen) {
    int i = 0;
    if (maxlen <= 0) return 0;

    while (1) {
        while (!usart_kbhit()) {
            _delay_ms(1);
        }
        char c = usart_getchar();

        /* CR/LF termina */
        if (c == '\r' || c == '\n') {
            /* echo newline */
            usart_putchar('\r');
            usart_putchar('\n');
            break;
        }

        /* Backspace (DEL or BS) */
        if (c == 127 || c == 8) {
            if (i > 0) {
                i--;
                /* erase on terminal */
                usart_putchar('\b');
                usart_putchar(' ');
                usart_putchar('\b');
            }
            continue;
        }

        /* normale */
        if (i < maxlen - 1) {
            buf[i++] = c;
            usart_putchar(c); /* echo */
        } else {
            /* buffer pieno: ignore additional chars but keep echoing */
            usart_putchar(c);
        }
    }
    buf[i] = '\0';
    return i;
}

// --- USART Related ---
void usart_init(uint16_t ubrr) {
    // Set baud rate
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data

    // Enable RX and TX, do NOT enable RX complete interrupt (no ISR provided)
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);

    // Small delay to let line settle and flush any spurious bytes present at power-up
    _delay_ms(5);
    // Drain any pending RX bytes (noise) so they don't appear as garbage in console
    while (UCSR0A & _BV(RXC0)) {
        volatile uint8_t dummy = UDR0;
        (void)dummy;
        // short pause in case multiple bytes
        _delay_ms(1);
    }
}

void usart_putchar(char data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (_BV(UDRE0))));
    // Start transmission
    UDR0 = data;
}

char usart_getchar(void) {
    // Wait for incoming data
    while (!(UCSR0A & (_BV(RXC0))));
    // Return the data
    return UDR0;
}

unsigned char usart_kbhit(void) {
    // Return nonzero if char waiting (polled version)
    unsigned char b = 0;
    if (UCSR0A & (1 << RXC0)) b = 1;
    return b;
}

void usart_pstr(char *s) {
    // Loop through entire string
    while (*s) {
        usart_putchar(*s);
        s++;
    }
}

// Called by printf as a stream handler
int usart_putchar_printf(char var, FILE *stream) {
    // Translate \n to \r\n for terminal compatibility
    if (var == '\n') usart_putchar('\r');
    usart_putchar(var);
    return 0;
}

void printf_init(void) {
    stdout = &mystdout;

    // Fire up the USART
    usart_init(MYUBRR);
}





