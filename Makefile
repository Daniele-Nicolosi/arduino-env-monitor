# ------------------------------------------------------------
#  Makefile principale del progetto Arduino I2C Proxy
#  Gestisce la compilazione del firmware (Arduino)
#  e del client (programma PC)
# ------------------------------------------------------------

.PHONY: all clean firmware client

# ------------------------------------------------------------
#  Target predefinito: compila firmware + client
# ------------------------------------------------------------
all: firmware client

# ------------------------------------------------------------
#  Compilazione del firmware per Arduino
# ------------------------------------------------------------
firmware:
	@echo "🔧 Compilazione firmware Arduino..."
	$(MAKE) -C src

# ------------------------------------------------------------
#  Compilazione del client per PC
# ------------------------------------------------------------
client:
	@echo "💻 Compilazione client PC..."
	$(MAKE) -C client

# ------------------------------------------------------------
#  Pulizia completa del progetto
# ------------------------------------------------------------
clean:
	@echo "🧹 Pulizia di firmware e client..."
	$(MAKE) -C src clean
	$(MAKE) -C client clean
























