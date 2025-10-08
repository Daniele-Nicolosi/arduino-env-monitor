#include "../../avr_common/i2c/i2c.h"
#include "bme280.h"

/* ------------------------------------------------------------
   Coefficienti di calibrazione interni
   (letti una sola volta da memoria non volatile del BME280)
------------------------------------------------------------ */
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;

static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

static uint8_t  dig_H1;
static int16_t  dig_H2;
static uint8_t  dig_H3;
static int16_t  dig_H4, dig_H5;
static int8_t   dig_H6;

static int32_t t_fine;   // variabile di calibrazione temperatura

/* ------------------------------------------------------------
   Funzioni di supporto (lettura registri via I2C)
------------------------------------------------------------ */
static uint16_t bme280_read16(uint8_t reg) {
    uint8_t buf[2];
    i2c_read_regs(BME280_ADDR, reg, buf, 2);
    return ((uint16_t)buf[1] << 8) | buf[0];
}

static int16_t bme280_readS16(uint8_t reg) {
    return (int16_t)bme280_read16(reg);
}

static int32_t bme280_read_raw_temp(void) {
    uint8_t buf[3];
    i2c_read_regs(BME280_ADDR, 0xFA, buf, 3);
    return ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | (buf[2] >> 4);
}

static int32_t bme280_read_raw_press(void) {
    uint8_t buf[3];
    i2c_read_regs(BME280_ADDR, 0xF7, buf, 3);
    return ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | (buf[2] >> 4);
}

static int32_t bme280_read_raw_hum(void) {
    uint8_t buf[2];
    i2c_read_regs(BME280_ADDR, 0xFD, buf, 2);
    return ((int32_t)buf[0] << 8) | buf[1];
}

/* ------------------------------------------------------------
   bme280_init()
   Legge i coefficienti di calibrazione e configura il sensore:
   - Oversampling x1 per temperatura, pressione e umidità
   - Modalità "normal"
   - Imposta i registri di configurazione base
------------------------------------------------------------ */
void bme280_init(void) {
    // ---- Coefficienti calibrazione temperatura ----
    dig_T1 = bme280_read16(0x88);
    dig_T2 = bme280_readS16(0x8A);
    dig_T3 = bme280_readS16(0x8C);

    // ---- Coefficienti calibrazione pressione ----
    dig_P1 = bme280_read16(0x8E);
    dig_P2 = bme280_readS16(0x90);
    dig_P3 = bme280_readS16(0x92);
    dig_P4 = bme280_readS16(0x94);
    dig_P5 = bme280_readS16(0x96);
    dig_P6 = bme280_readS16(0x98);
    dig_P7 = bme280_readS16(0x9A);
    dig_P8 = bme280_readS16(0x9C);
    dig_P9 = bme280_readS16(0x9E);

    // ---- Coefficienti calibrazione umidità ----
    {
        uint8_t tmp;
        i2c_read_reg(BME280_ADDR, 0xA1, &tmp);
        dig_H1 = tmp;
    }

    dig_H2 = bme280_readS16(0xE1);

    {
        uint8_t tmp;
        i2c_read_reg(BME280_ADDR, 0xE3, &tmp);
        dig_H3 = tmp;
    }

    {
        uint8_t e4, e5, e6;
        i2c_read_reg(BME280_ADDR, 0xE4, &e4);
        i2c_read_reg(BME280_ADDR, 0xE5, &e5);
        i2c_read_reg(BME280_ADDR, 0xE6, &e6);
        dig_H4 = (int16_t)((e4 << 4) | (e5 & 0x0F));
        dig_H5 = (int16_t)((e6 << 4) | (e5 >> 4));
    }

    {
        uint8_t tmp;
        i2c_read_reg(BME280_ADDR, 0xE7, &tmp);
        dig_H6 = (int8_t)tmp;
    }

    // ---- Configurazione sensore ----
    i2c_write_reg(BME280_ADDR, 0xF2, 0x01);   // ctrl_hum: oversampling x1
    i2c_write_reg(BME280_ADDR, 0xF4, 0x27);   // ctrl_meas: temp+press x1, normal mode
    i2c_write_reg(BME280_ADDR, 0xF5, 0xA0);   // config: standby 1000ms
}

/* ------------------------------------------------------------
   bme280_read_temperature()
   Legge la temperatura compensata in °C
   - Usa le formule Bosch originali con i coefficienti letti
   - Aggiorna la variabile t_fine (usata anche per P e H)
------------------------------------------------------------ */
float bme280_read_temperature(void) {
    int32_t adc_T = bme280_read_raw_temp();
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * (int32_t)dig_T2) >> 11;
    var2 = (((((adc_T >> 4) - (int32_t)dig_T1) *
              ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) *
            (int32_t)dig_T3) >> 14;
    t_fine = var1 + var2;
    return ((t_fine * 5 + 128) >> 8) / 100.0f;
}

/* ------------------------------------------------------------
   bme280_read_pressure()
   Legge la pressione compensata in hPa
   - Richiede t_fine calcolato in precedenza
   - Esegue la formula di compensazione intera a 64 bit
------------------------------------------------------------ */
float bme280_read_pressure(void) {
    int32_t adc_P = bme280_read_raw_press();
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * (int64_t)dig_P1) >> 33;
    if (var1 == 0) return 0.0f;  // protezione da divisione per zero
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (float)p / 25600.0f;  // hPa
}

/* ------------------------------------------------------------
   bme280_read_humidity()
   Legge l’umidità relativa compensata in %RH
   - Richiede t_fine calcolato dalla temperatura
   - Applica compensazione secondo datasheet Bosch
------------------------------------------------------------ */
float bme280_read_humidity(void) {
    int32_t adc_H = bme280_read_raw_hum();
    int32_t v_x1_u32r;
    v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((adc_H << 14) - ((int32_t)dig_H4 << 20) -
                    ((int32_t)dig_H5 * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * (int32_t)dig_H6) >> 10) *
                      (((v_x1_u32r * (int32_t)dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
                   (int32_t)dig_H2 + 8192) >> 14));
    v_x1_u32r = v_x1_u32r -
                (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                  (int32_t)dig_H1) >> 4);
    if (v_x1_u32r < 0) v_x1_u32r = 0;
    if (v_x1_u32r > 419430400) v_x1_u32r = 419430400;
    return (v_x1_u32r >> 12) / 1024.0f;
}





