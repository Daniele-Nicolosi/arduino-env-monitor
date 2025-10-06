#pragma once

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* ------------------------------------------------------------
   Configurazione UART
------------------------------------------------------------ */
#define UART_BAUD        19200
#define UART_RX_BUF_SIZE 64
#define UART_TX_BUF_SIZE 64
#define UART_MYUBRR (F_CPU / 16 / UART_BAUD - 1)

/* ------------------------------------------------------------
   API UART (interrupt-driven)
------------------------------------------------------------ */
void UART_init(uint16_t ubrr);
void UART_putChar(char data);
char UART_getChar(void);
void UART_putString(const char *s);
int  UART_getString(char *buf, int maxlen);


















