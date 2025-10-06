# ------------------------------------------------------------
#  Makefile per il firmware Arduino (cartella src)
#  Usa il file comune ../avr_common/avr.mk
# ------------------------------------------------------------

# Nome del file eseguibile per Arduino
BINS = main.hex

# ------------------------------------------------------------
#  Oggetti da compilare
# ------------------------------------------------------------
OBJS = proxy/proxy.o \
       ../avr_common/uart/uart.o \
       ../avr_common/i2c/i2c.o \
       sensors/bme280.o \
       display/oled.o \
       display/font/font.o

# ------------------------------------------------------------
#  Header (opzionali, per chiarezza e dipendenze)
# ------------------------------------------------------------
HEADERS = proxy/proxy.h \
          ../avr_common/uart/uart.h \
          ../avr_common/i2c/i2c.h \
          sensors/bme280.h \
          display/oled.h \
          display/font/font.h

# ------------------------------------------------------------
#  Include il Makefile comune per la toolchain AVR
#  (contiene le regole per compilazione, linking e upload)
# ------------------------------------------------------------
include ../avr_common/avr.mk





