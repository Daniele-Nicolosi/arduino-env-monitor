#pragma once

#include <stdio.h>

// --- Function Prototypes ---

// Inizializza printf sulla UART
void printf_init(void);

// Funzioni di base della UART
void usart_putchar(char data);
char usart_getchar(void);
unsigned char usart_kbhit(void);
int usart_putchar_printf(char var, FILE *stream);

// Utility di invio stringhe
void uart_puts(const char *s);

// Legge una linea dalla UART (eco dei caratteri, gestisce backspace).
// Restituisce numero di caratteri (terminazione nulla garantita).
int uart_readline(char *buf, int maxlen);





