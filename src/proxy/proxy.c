#include <stdio.h>
#include <stdlib.h>     // dtostrf
#include <string.h>
#include <ctype.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../../avr_common/uart/uart.h"
#include "../../avr_common/i2c/i2c.h"
#include "../sensors/bme280.h"
#include "../display/oled.h"

// --- Funzioni di supporto ---

// Converte una stringa in minuscolo (in place)
static void str_to_lower(char *s) {
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

// --- Inizializzazione del proxy ---
void proxy_init(void) {
    printf_init();   // UART con printf
    i2c_init();      // I2C @100kHz
    bme280_init();   // Inizializza sensore e carica calibrazioni
    oled_init();   // inizializza OLED
}

// --- Loop principale del proxy ---
void proxy_run(void) {
    enum sensor_t { S_TEMP, S_PRESS, S_HUM };

    // Stampa il menu e le istruzioni una sola volta
    printf("\r\n---Sensor Reading---\r\n");
    printf("-) To read the temperature, type \"read temp\"\r\n");
    printf("-) To read the pressure, type \"read press\"\r\n");
    printf("-) To read the humidity, type \"read hum\"\r\n");
    printf("-) To exit, type \"q\"\r\n");
    printf("It is possible to read multiple values, e.g.: \"read temp press hum\"\r\n");

    int running = 1;
    while (running) {
        printf("Please enter the parameter you want to read:\n ---> ");

        // --- Leggi input da UART ---
        char line[128];
        int len = uart_readline(line, sizeof(line));
        if (len == 0) continue;

        printf("\n");

        // Uscita rapida
        if ((len == 1) && (line[0] == 'q' || line[0] == 'Q')) {
            printf("Exiting...\r\n");
            break;
        }

        // --- Parsing input: supporta "read temp press hum" e "temp press" ---
        char *saveptr = NULL;
        char *tok = NULL;
        int requests[3];
        int req_count = 0;

        tok = strtok_r(line, " \t,;:", &saveptr);
        while (tok != NULL && req_count < 3) {
            char word[32];
            strncpy(word, tok, sizeof(word)-1);
            word[sizeof(word)-1] = '\0';
            str_to_lower(word);

            // skip the keyword "read"
            if (strcmp(word, "read") == 0) {
                tok = strtok_r(NULL, " \t,;:", &saveptr);
                continue;
            }

            if (strcmp(word, "temp") == 0 || strcmp(word, "temperature") == 0) {
                requests[req_count++] = S_TEMP;
            } else if (strcmp(word, "press") == 0 || strcmp(word, "pressure") == 0) {
                requests[req_count++] = S_PRESS;
            } else if (strcmp(word, "hum") == 0 || strcmp(word, "humidity") == 0) {
                requests[req_count++] = S_HUM;
            } else if (strcmp(word, "q") == 0 || strcmp(word, "quit") == 0) {
                running = 0;
                break;
            }
            tok = strtok_r(NULL, " \t,;:", &saveptr);
        }

        if (!running) break;

        if (req_count == 0) {
            printf("Invalid command. Please try again.\r\n");
            oled_clear();
            oled_print_line(1, "   INVALID COMMAND");
            oled_print_line(5, "   Please try again");
            continue;
        }

        // --- Lettura sensore ---
        // Leggi temperatura per prima per aggiornare t_fine (necessario per pressione/umidita')
        float t_cached = bme280_read_temperature();

        char tbuf[32], pbuf[32], hbuf[32];

        // Reset buffer
        strcpy(tbuf, "Temperature:");
        strcpy(pbuf, "Pressure:");
        strcpy(hbuf, "Humidity:");

        // Stampa ogni valore su seriale e prepara stringhe OLED
        for (int i = 0; i < req_count; ++i) {
            switch (requests[i]) {
                case S_TEMP: {
                    char val[16];
                    dtostrf(t_cached, 6, 2, val);
                    printf("Temperature: %s C\r\n", val);
                    snprintf(tbuf, sizeof(tbuf), "Temperature: %s C", val);
                    break;
                }
                case S_PRESS: {
                    float p = bme280_read_pressure();
                    char val[16];
                    dtostrf(p, 7, 2, val);
                    printf("Pressure: %s hPa\r\n", val);
                    snprintf(pbuf, sizeof(pbuf), "Pressure: %s hPa", val);
                    break;
                }
                case S_HUM: {
                    float h = bme280_read_humidity();
                    char val[16];
                    dtostrf(h, 6, 2, val);
                    printf("Humidity: %s %%\r\n", val);
                    snprintf(hbuf, sizeof(hbuf), "Humidity: %s %%", val);
                    break;
                }
                default:
                    break;
            }
        }
        // Aggiorna display OLED
        oled_show_sensors(tbuf, pbuf, hbuf);
    }
}








