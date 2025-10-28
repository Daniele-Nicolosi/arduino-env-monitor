#pragma once

#include <stdint.h>

/* ------------------------------------------------------------
   Indirizzo I2C del sensore BME280
------------------------------------------------------------ */
#define BME280_ADDR 0x76

/* ------------------------------------------------------------
   BME280_init()
   Inizializza il sensore:
   - Legge i coefficienti di calibrazione interni
   - Configura oversampling e modalità normale
------------------------------------------------------------ */
void BME280_init(void);

/* ------------------------------------------------------------
   Imposta il tempo di standby (sampling rate interno)
------------------------------------------------------------ */
void BME280_set_sampling(uint16_t ms);

/* ------------------------------------------------------------
   Letture dei parametri ambientali 
   Restituiscono valori già compensati tramite formule Bosch.
   Unità:
   - Temperatura: °C
   - Pressione:   hPa
   - Umidità:     %RH
------------------------------------------------------------ */
float BME280_read_temperature(void);
float BME280_read_pressure(void);
float BME280_read_humidity(void);




