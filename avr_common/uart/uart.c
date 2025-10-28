#include "uart.h"

/* ------------------------------------------------------------
   Buffer circolari 
------------------------------------------------------------ */
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE]; // buffer di ricezione
static volatile uint8_t rx_head = 0, rx_tail = 0;

static volatile uint8_t tx_buf[UART_TX_BUF_SIZE]; // buffer di trasmissione 
static volatile uint8_t tx_head = 0, tx_tail = 0;

/* ------------------------------------------------------------
   UART_init()
   Configura la UART: 19200 baud, 8N1
------------------------------------------------------------ */
void UART_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bit, no parity, 1 stop
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) |  // abilita RX e TX
             (1 << RXCIE0);                 // abilita interrupt RX

    sei(); // abilita interrupt globali
}

/* ------------------------------------------------------------
   UART_putChar()
   Inserisce un carattere nel buffer TX e abilita interrupt TX
------------------------------------------------------------ */
void UART_putChar(char data) {
    uint8_t next = (tx_head + 1) % UART_TX_BUF_SIZE;
    while (next == tx_tail); // il buffer è pieno, attende spazio libero

    tx_buf[tx_head] = data;
    tx_head = next;

    UCSR0B |= (1 << UDRIE0); // abilita interrupt "data register empty" (UDR0 vuoto)
}

/* ------------------------------------------------------------
   UART_getChar()
   Ritorna il primo carattere nel buffer RX (bloccante)
------------------------------------------------------------ */
char UART_getChar(void) {
    while (rx_head == rx_tail); // attende dati disponibili
    char c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % UART_RX_BUF_SIZE;
    return c;
}

/* ------------------------------------------------------------
   UART_putString()
   Invia una stringa 
------------------------------------------------------------ */
void UART_putString(const char *s) {
    while (*s) UART_putChar(*s++);
}

/* ------------------------------------------------------------
   UART_getString()
   Legge una riga con terminatore '\r' o '\n'
------------------------------------------------------------ */
int UART_getString(char *buf, int maxlen) {
    int i = 0;
    char c;
    while (i < maxlen - 1) {
        c = UART_getChar();
        if (c == '\r' || c == '\n') {
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

/* ------------------------------------------------------------
   ISR: Ricezione (USART0_RX_vect)
   Viene chiamata automaticamente quando arriva un byte
------------------------------------------------------------ */
ISR(USART0_RX_vect) {
    uint8_t data = UDR0;
    uint8_t next = (rx_head + 1) % UART_RX_BUF_SIZE;

    if (next != rx_tail) { // Controlla se il buffer non è pieno
        rx_buf[rx_head] = data;
        rx_head = next;
    }
}

/* ------------------------------------------------------------
   ISR: Trasmissione (USART0_UDRE_vect)
   Invia un byte dal buffer TX quando il registro è vuoto
------------------------------------------------------------ */
ISR(USART0_UDRE_vect) {
    if (tx_head == tx_tail) {
        UCSR0B &= ~(1 << UDRIE0); // disabilita interrupt se buffer vuoto
    } else {
        UDR0 = tx_buf[tx_tail];
        tx_tail = (tx_tail + 1) % UART_TX_BUF_SIZE;
    }
}





















