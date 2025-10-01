#include "../../avr_common/i2c/i2c.h"
#include "bme280.h"
#include <util/delay.h>

#define BME280_ADDR 0x76

static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static int32_t t_fine;

static uint16_t bme280_read16(uint8_t reg) {
    uint8_t buf[2];
    if (i2c_read_regs(BME280_ADDR, reg, buf, 2)) {
        return 0;
    }
    return ((uint16_t)buf[1] << 8) | buf[0];
}

static int16_t bme280_readS16(uint8_t reg) {
    return (int16_t)bme280_read16(reg);
}

static int32_t bme280_read_raw_temp(void) {
    uint8_t buf[3];
    if (i2c_read_regs(BME280_ADDR, 0xFA, buf, 3)) {
        return 0;
    }
    return ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | (buf[2] >> 4);
}

void bme280_init(void) {
    // Lettura parametri di calibrazione temperatura
    dig_T1 = bme280_read16(0x88);
    dig_T2 = bme280_readS16(0x8A);
    dig_T3 = bme280_readS16(0x8C);

    // Configurazione base: oversampling 1x, mode = normal
    i2c_write_reg(BME280_ADDR, 0xF2, 0x01);   // ctrl_hum
    i2c_write_reg(BME280_ADDR, 0xF4, 0x27);   // ctrl_meas
    i2c_write_reg(BME280_ADDR, 0xF5, 0xA0);   // config
}

float bme280_read_temperature(void) {
    int32_t adc_T = bme280_read_raw_temp();
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - (int32_t)dig_T1) *
              ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) *
            (int32_t)dig_T3) >> 14;
    t_fine = var1 + var2;
    return ((t_fine * 5 + 128) >> 8) / 100.0f;
}


