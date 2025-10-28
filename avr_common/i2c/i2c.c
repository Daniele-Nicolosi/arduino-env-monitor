#include "i2c.h"

/* ------------------------------------------------------------
   I2C_init()
   Inizializza l'interfaccia I2C in modalità Master.
   Configura il prescaler e il bitrate per 100 kHz (F_CPU = 16 MHz)
------------------------------------------------------------ */
void I2C_init(void) {
    TWSR = 0x00;                                   // Prescaler = 1
    TWBR = ((F_CPU / 100000UL) - 16) / 2;          // Bitrate = 100 kHz
}

/* ------------------------------------------------------------
   I2C_start()
   Invia una condizione START e l'indirizzo dello slave
   device_addr: indirizzo a 7 bit dello slave
   mode: I2C_WRITE (0) o I2C_READ (1)
   Ritorna: codice di stato TWI (registri TWSR)
------------------------------------------------------------ */
uint8_t I2C_start(uint8_t device_addr, uint8_t mode) {
    // Invia condizione START
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT))); // Aspetta il completamento (TWINT settato a 1)

    // Invia indirizzo + bit R/W
    TWDR = (device_addr << 1) | (mode & 0x01);
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));

    return (TWSR & 0xF8); // Unicamente i primi 5 bit sono significativi
}

/* ------------------------------------------------------------
   I2C_stop()
   Invia una condizione STOP e rilascia il bus
------------------------------------------------------------ */
void I2C_stop(void) {
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
    _delay_us(10); 
}

/* ------------------------------------------------------------
   I2C_write()
   Invia un byte di dati allo slave
   Ritorna: codice di stato TWI (TWSR)
------------------------------------------------------------ */
uint8_t I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
    return (TWSR & 0xF8);
}

/* ------------------------------------------------------------
   I2C_read_ack()
   Legge un byte e invia ACK (continua la lettura)
------------------------------------------------------------ */
uint8_t I2C_read_ack(void) {
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA); // ACK, altri byte da leggere
    while (!(TWCR & (1 << TWINT)));
    return TWDR;  // Ritorna il byte letto
}

/* ------------------------------------------------------------
   I2C_read_nack()
   Legge un byte e invia NACK (termina la lettura)
------------------------------------------------------------ */
uint8_t I2C_read_nack(void) {
    TWCR = (1 << TWEN) | (1 << TWINT); // NACK, ultimo byte da leggere
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

/* ------------------------------------------------------------
   I2C_write_reg()
   Scrive un byte in un registro dello slave
   dev: indirizzo dispositivo
   reg: registro
   val: valore da scrivere
   Ritorna 0 in caso di successo, codice di errore altrimenti
------------------------------------------------------------ */
uint8_t I2C_write_reg(uint8_t dev, uint8_t reg, uint8_t val) {
    uint8_t st;
    st = I2C_start(dev, I2C_WRITE);  if (st != 0x18) { I2C_stop(); return st; }
    st = I2C_write(reg);             if (st != 0x28) { I2C_stop(); return st; }
    st = I2C_write(val);             if (st != 0x28) { I2C_stop(); return st; }
    I2C_stop();
    return 0;
}

/* ------------------------------------------------------------
   I2C_read_reg()
   Legge un byte da un registro dello slave
   dev: indirizzo dispositivo
   reg: registro da leggere
   out: puntatore alla variabile dove salvare il risultato
   Ritorna 0 in caso di successo, codice di errore altrimenti
------------------------------------------------------------ */
uint8_t I2C_read_reg(uint8_t dev, uint8_t reg, uint8_t *out) {
    uint8_t st;
    st = I2C_start(dev, I2C_WRITE);  if (st != 0x18) { I2C_stop(); return st; }
    st = I2C_write(reg);             if (st != 0x28) { I2C_stop(); return st; }
    st = I2C_start(dev, I2C_READ);   if (st != 0x40) { I2C_stop(); return st; }
    *out = I2C_read_nack();
    I2C_stop();
    return 0;
}

/* ------------------------------------------------------------
   I2C_read_regs()
   Legge più registri consecutivi (es. sensori come BME280)
   dev: indirizzo dispositivo
   start_reg: primo registro da leggere
   buf: buffer dove salvare i dati
   len: numero di byte da leggere
------------------------------------------------------------ */
uint8_t I2C_read_regs(uint8_t dev, uint8_t start_reg, uint8_t *buf, uint8_t len) {
    uint8_t st;
    st = I2C_start(dev, I2C_WRITE);  if (st != 0x18) { I2C_stop(); return st; }
    st = I2C_write(start_reg);       if (st != 0x28) { I2C_stop(); return st; }
    st = I2C_start(dev, I2C_READ);   if (st != 0x40) { I2C_stop(); return st; }

    for (uint8_t i = 0; i < len - 1; ++i)
        buf[i] = I2C_read_ack();

    buf[len - 1] = I2C_read_nack();
    I2C_stop();
    return 0;
}



