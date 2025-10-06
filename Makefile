# Quale eseguibile generare
BINS=src/main.elf

# Oggetti da compilare (metti qui i tuoi .c come .o)
OBJS= src/proxy/proxy.o avr_common/uart/uart.o avr_common/i2c/i2c.o src/sensors/bme280.o src/display/oled.o src/display/font/font.o

# Header coinvolti
HEADERS=src/proxy/proxy.h avr_common/uart/uart.h avr_common/i2c/i2c.h src/sensors/bme280.h src/display/oled.h src/display/font/font.h

flash: src/main.hex
	@echo "Caricato su Arduino Mega!"

# Include le regole del prof
include avr_common/avr.mk









