#pragma once

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* ------------------------------------------------------------
   Configura parametri della porta seriale (baudrate, 8N1, raw mode)
------------------------------------------------------------ */
int serial_set_interface_attribs(int fd, int speed);

/* ------------------------------------------------------------
   Apre il dispositivo seriale in lettura/scrittura
------------------------------------------------------------ */
int serial_open(const char* device);
