#include "i2c.h"

void i2c_init(void) {
    // Prescaler = 1
    TWSR = 0x00;
    // Bitrate = 100kHz (con F_CPU=16MHz)
    TWBR = ((F_CPU/100000UL)-16)/2;
}

uint8_t i2c_start(uint8_t device_addr, uint8_t mode) {
    // START condition
    TWCR = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);
    while(!(TWCR & (1<<TWINT)));

    // Invia indirizzo + bit R/W
    TWDR = (device_addr << 1) | (mode & 0x01);
    TWCR = (1<<TWEN)|(1<<TWINT);
    while(!(TWCR & (1<<TWINT)));

    return (TWSR & 0xF8); // codice stato
}

void i2c_stop(void) {
    TWCR = (1<<TWSTO)|(1<<TWEN)|(1<<TWINT);
    _delay_us(10); // attesa minima
}

uint8_t i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWEN)|(1<<TWINT);
    while(!(TWCR & (1<<TWINT)));
    return (TWSR & 0xF8);
}

uint8_t i2c_read_ack(void) {
    TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWEA); // ACK
    while(!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack(void) {
    TWCR = (1<<TWEN)|(1<<TWINT); // NACK
    while(!(TWCR & (1<<TWINT)));
    return TWDR;
}

// ***** Utility di alto livello *****

uint8_t i2c_write_reg(uint8_t dev, uint8_t reg, uint8_t val) {
    uint8_t st;
    st = i2c_start(dev, I2C_WRITE);  if (st!=0x18) { i2c_stop(); return st; }
    st = i2c_write(reg);             if (st!=0x28) { i2c_stop(); return st; }
    st = i2c_write(val);             if (st!=0x28) { i2c_stop(); return st; }
    i2c_stop();
    return 0; // successo
}

uint8_t i2c_read_reg(uint8_t dev, uint8_t reg, uint8_t *out) {
    uint8_t st;
    st = i2c_start(dev, I2C_WRITE);  if (st!=0x18) { i2c_stop(); return st; }
    st = i2c_write(reg);             if (st!=0x28) { i2c_stop(); return st; }
    st = i2c_start(dev, I2C_READ);   if (st!=0x40) { i2c_stop(); return st; }
    *out = i2c_read_nack();
    i2c_stop();
    return 0;
}

uint8_t i2c_read_regs(uint8_t dev, uint8_t start_reg, uint8_t *buf, uint8_t len) {
    uint8_t st;
    st = i2c_start(dev, I2C_WRITE);  if (st!=0x18) { i2c_stop(); return st; }
    st = i2c_write(start_reg);       if (st!=0x28) { i2c_stop(); return st; }
    st = i2c_start(dev, I2C_READ);   if (st!=0x40) { i2c_stop(); return st; }
    for (uint8_t i=0; i<len-1; ++i) buf[i] = i2c_read_ack();
    buf[len-1] = i2c_read_nack();
    i2c_stop();
    return 0;
}

