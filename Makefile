# Quale eseguibile generare
BINS=src/main.elf

# Oggetti da compilare (metti qui i tuoi .c come .o)
OBJS= src/proxy.o avr_common/uart.o avr_common/i2c.o

# Header coinvolti
HEADERS=src/proxy.h avr_common/uart.h avr_common/i2c.h

flash: src/main.hex
	@echo "Caricato su Arduino Mega!"

# Include le regole del prof
include avr_common/avr.mk

