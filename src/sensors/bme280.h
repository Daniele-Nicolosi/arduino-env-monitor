#pragma once

#include <stdint.h>

#define BME280_ADDR 0x76

// Inizializza il BME280 (calibrazione + settaggi base)
void bme280_init(void);

// Letture in floating point già convertite
float bme280_read_temperature(void);  // °C
float bme280_read_pressure(void);     // hPa
float bme280_read_humidity(void);     // %RH


