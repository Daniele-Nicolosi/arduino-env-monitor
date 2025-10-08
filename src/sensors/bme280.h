#pragma once

#include <stdint.h>

/* ------------------------------------------------------------
   Indirizzo I2C del sensore BME280
------------------------------------------------------------ */
#define BME280_ADDR 0x76

/* ------------------------------------------------------------
   bme280_init()
   Inizializza il sensore:
   - Legge i coefficienti di calibrazione interni
   - Configura oversampling e modalità normale
------------------------------------------------------------ */
void bme280_init(void);

/* ------------------------------------------------------------
   Letture dei parametri ambientali 
   Restituiscono valori già compensati tramite formule Bosch.
   Unità:
   - Temperatura: °C
   - Pressione:   hPa
   - Umidità:     %RH
------------------------------------------------------------ */
float bme280_read_temperature(void);
float bme280_read_pressure(void);
float bme280_read_humidity(void);



