#include <stdlib.h>     // dtostrf
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../../avr_common/uart/uart.h"
#include "../../avr_common/i2c/i2c.h"
#include "../sensors/bme280.h"
#include "../display/oled.h"

/* ------------------------------------------------------------
   Converte una stringa in minuscolo (in place)
------------------------------------------------------------ */
static void str_to_lower(char *s) {
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

/* ------------------------------------------------------------
   Inizializzazione del proxy
------------------------------------------------------------ */
void proxy_init(void) {
    UART_init(UART_MYUBRR); // UART basata su interrupt
    i2c_init();             // I2C @100kHz
    bme280_init();          // Inizializza sensore e calibrazioni
    oled_init();            // Inizializza OLED
}

/* ------------------------------------------------------------
   Loop principale del proxy
------------------------------------------------------------ */
void proxy_run(void) {
    enum sensor_t { S_TEMP, S_PRESS, S_HUM };

    // Messaggio di benvenuto
    UART_putString("\r\n--- Sensor Reading ---\r\n");
    UART_putString("-) To read the temperature, type \"read temp\"\r\n");
    UART_putString("-) To read the pressure, type \"read press\"\r\n");
    UART_putString("-) To read the humidity, type \"read hum\"\r\n");
    UART_putString("-) To exit, type \"q\"\r\n");
    UART_putString("It is possible to read multiple values, e.g.: \"read temp press hum\"\r\n");

    int running = 1;
    char line[128];

    while (running) {
        UART_putString("Please enter the parameter you want to read:\r\n ---> ");
        int len = UART_getString(line, sizeof(line));
        if (len == 0) continue;

        UART_putString("\r\n");

        // Uscita rapida
        if ((len == 1) && (line[0] == 'q' || line[0] == 'Q')) {
            UART_putString("Exiting...\r\n");
            break;
        }

        // --- Parsing input ---
        char *saveptr = NULL;
        char *tok = NULL;
        int requests[3];
        int req_count = 0;

        tok = strtok_r(line, " \t,;:", &saveptr);
        while (tok != NULL && req_count < 3) {
            char word[32];
            strncpy(word, tok, sizeof(word) - 1);
            word[sizeof(word) - 1] = '\0';
            str_to_lower(word);

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
            UART_putString("Invalid command. Please try again.\r\n");
            oled_clear();
            oled_print_line(1, "   INVALID COMMAND");
            oled_print_line(5, "   Please try again");
            continue;
        }

        // --- Lettura sensore ---
        float t_cached = bme280_read_temperature();

        char tbuf[32], pbuf[32], hbuf[32];
        strcpy(tbuf, "Temperature:");
        strcpy(pbuf, "Pressure:");
        strcpy(hbuf, "Humidity:");

        for (int i = 0; i < req_count; ++i) {
            char val[16], msg[64];

            switch (requests[i]) {
                case S_TEMP: {
                    dtostrf(t_cached, 6, 2, val);
                    snprintf(msg, sizeof(msg), "Temperature: %s C\r\n", val);
                    UART_putString(msg);
                    snprintf(tbuf, sizeof(tbuf), "Temperature: %s C", val);
                    break;
                }
                case S_PRESS: {
                    float p = bme280_read_pressure();
                    dtostrf(p, 7, 2, val);
                    snprintf(msg, sizeof(msg), "Pressure: %s hPa\r\n", val);
                    UART_putString(msg);
                    snprintf(pbuf, sizeof(pbuf), "Pressure: %s hPa", val);
                    break;
                }
                case S_HUM: {
                    float h = bme280_read_humidity();
                    dtostrf(h, 6, 2, val);
                    snprintf(msg, sizeof(msg), "Humidity: %s %%\r\n", val);
                    UART_putString(msg);
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









