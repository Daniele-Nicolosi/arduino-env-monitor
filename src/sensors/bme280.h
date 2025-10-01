#pragma once
#include <stdint.h>

#define BME280_ADDR 0x76

void bme280_init(void);
float bme280_read_temperature(void);

