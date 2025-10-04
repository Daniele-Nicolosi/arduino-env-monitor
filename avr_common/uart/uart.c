#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BAUD 19200
#define MYUBRR F_CPU/16/BAUD-1

#define UART_RX_BUF_SIZE 64
#define UART_TX_BUF_SIZE 64

static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];
static volatile uint8_t rx_head = 0, rx_tail = 0;

static volatile uint8_t tx_buf[UART_TX_BUF_SIZE];
static volatile uint8_t tx_head = 0, tx_tail = 0;

static int usart_putchar_printf(char c, FILE *stream);

/* stream per printf */
static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

/* --- Inizializzazione USART --- */
void usart_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);     // 8-bit
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) |      // abilita RX e TX
             _BV(RXCIE0);                   // abilita interrupt RX

    _delay_ms(5);
}

/* init stdio -> printf usa UART */
void printf_init(void) {
    stdout = &mystdout;
    usart_init(MYUBRR);
    sei();  // abilita interrupt globali
}

/* --- Trasmissione --- */
void usart_putchar(char data) {
    uint8_t next = (tx_head + 1) % UART_TX_BUF_SIZE;
    while (next == tx_tail);  // aspetta se buffer pieno

    tx_buf[tx_head] = data;
    tx_head = next;

    // abilita interrupt "data register empty"
    UCSR0B |= _BV(UDRIE0);
}

/* Funzione compatibile con printf */
static int usart_putchar_printf(char c, FILE *stream) {
    if (c == '\n') usart_putchar('\r');
    usart_putchar(c);
    return 0;
}

/* --- Ricezione --- */
char usart_getchar(void) {
    while (rx_head == rx_tail);  // aspetta carattere
    char c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % UART_RX_BUF_SIZE;
    return c;
}

unsigned char usart_kbhit(void) {
    return (rx_head != rx_tail);
}

/* --- ISR --- */
// Ricezione: nuovo byte da UART -> buffer circolare
ISR(USART0_RX_vect) {
    uint8_t data = UDR0;
    uint8_t next = (rx_head + 1) % UART_RX_BUF_SIZE;
    if (next != rx_tail) {
        rx_buf[rx_head] = data;
        rx_head = next;
    }
}

// Trasmissione: Data Register Empty
ISR(USART0_UDRE_vect) {
    if (tx_head == tx_tail) {
        // buffer vuoto -> disabilita interrupt
        UCSR0B &= ~_BV(UDRIE0);
    } else {
        UDR0 = tx_buf[tx_tail];
        tx_tail = (tx_tail + 1) % UART_TX_BUF_SIZE;
    }
}

/* --- Utility --- */
void uart_puts(const char *s) {
    while (*s) usart_putchar(*s++);
}

/* readline con echo (usa RX da interrupt + TX da interrupt) */
int uart_readline(char *buf, int maxlen) {
    int i = 0;
    while (1) {
        while (!usart_kbhit());  // aspetta input
        char c = usart_getchar();

        if (c == '\r' || c == '\n') {
            usart_putchar('\r');
            usart_putchar('\n');
            break;
        }

        if ((c == 127 || c == 8) && i > 0) {
            i--;
            usart_putchar('\b'); usart_putchar(' '); usart_putchar('\b');
            continue;
        }

        if (i < maxlen - 1) {
            buf[i++] = c;
            usart_putchar(c);
        }
    }
    buf[i] = '\0';
    return i;
}


















