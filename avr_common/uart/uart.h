#pragma once

#include <stdint.h>
#include <stdio.h>

void printf_init(void);
void uart_puts(const char *s);
int uart_readline(char *buf, int maxlen);















