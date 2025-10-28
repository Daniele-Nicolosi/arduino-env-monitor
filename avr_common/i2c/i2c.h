#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* ------------------------------------------------------------
   Costanti e modalità di trasferimento I2C
------------------------------------------------------------ */
#define I2C_WRITE  0
#define I2C_READ   1

/* ------------------------------------------------------------
   Inizializzazione
------------------------------------------------------------ */
void I2C_init(void);

/* ------------------------------------------------------------
   Primitive di basso livello (START, STOP, read/write singolo byte)
------------------------------------------------------------ */
uint8_t I2C_start(uint8_t device_addr, uint8_t mode);
void    I2C_stop(void);
uint8_t I2C_write(uint8_t data);
uint8_t I2C_read_ack(void);
uint8_t I2C_read_nack(void);

/* ------------------------------------------------------------
   Funzioni di alto livello (accesso a registri)
------------------------------------------------------------ */
// Scrive un byte in un registro di uno slave
uint8_t I2C_write_reg(uint8_t dev, uint8_t reg, uint8_t val);

// Legge un byte da un registro di uno slave
uint8_t I2C_read_reg(uint8_t dev, uint8_t reg, uint8_t *out);

// Legge più byte consecutivi (es. per sensori tipo BME280)
uint8_t I2C_read_regs(uint8_t dev, uint8_t start_reg, uint8_t *buf, uint8_t len);



