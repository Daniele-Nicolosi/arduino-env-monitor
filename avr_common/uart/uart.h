#pragma once

#include <stdint.h>

// ---- API UART ----
void usart_init(uint16_t ubrr);
void usart_putchar(char data);
char usart_getchar(void);
unsigned char usart_kbhit(void);
void usart_pstr(char *s);

// ---- Supporto printf ----
int usart_putchar_printf(char var, FILE *stream);
void printf_init(void);
