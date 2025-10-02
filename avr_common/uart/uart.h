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


