# ------------------------------------------------------------
#  Makefile per il client PC
#  Compila il programma che comunica con Arduino
# ------------------------------------------------------------

# Compilatore e flag
CC = gcc
CFLAGS = -Wall -O2

# File oggetto
OBJS = client.o

#File header
HEADERS = client.h

# ------------------------------------------------------------
#  Target predefinito: compila il client
# ------------------------------------------------------------
all: client

# ------------------------------------------------------------
#  Regola per creare l’eseguibile
# ------------------------------------------------------------
client: $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o client $(OBJS)

# ------------------------------------------------------------
#  Regola per compilare il file sorgente .c
# ------------------------------------------------------------
client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

# ------------------------------------------------------------
#  Pulizia dei file generati
# ------------------------------------------------------------
clean:
	rm -f *.o client

.PHONY: all clean






