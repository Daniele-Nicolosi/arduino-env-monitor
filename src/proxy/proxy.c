#include <stdio.h>
#include <stdlib.h>     // dtostrf
#include <string.h>
#include <ctype.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../../avr_common/uart/uart.h"
#include "../../avr_common/i2c/i2c.h"
#include "../sensors/bme280.h"

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
    sei();           // Abilita interrupt globali
    bme280_init();   // Inizializza sensore e carica calibrazioni
}

// --- Loop principale del proxy ---
void proxy_run(void) {
    enum sensor_t { S_TEMP, S_PRESS, S_HUM };

    // Stampa il menu e le istruzioni una sola volta
    printf("\r\n---Lettura sensore---\r\n");
    printf("-) Per poter leggere la temperatura, digitare \"read temp\"\r\n");
    printf("-) Per poter leggere la pressione, digitare \"read press\"\r\n");
    printf("-) Per poter leggere l'umidita', digitare \"read hum\"\r\n");
    printf("-) Per uscire, digitare \"q\"\r\n");
    printf("E' possibile leggere piu' valori, ad esempio: \"read temp press hum\"\r\n");

    int running = 1;
    while (running) {
        printf("Digitare il parametro che si desidera leggere ---> ");

        // --- Leggi input da UART ---
        char line[128];
        int len = uart_readline(line, sizeof(line));
        if (len == 0) continue;

        printf("\n");

        // Uscita rapida
        if ((len == 1) && (line[0] == 'q' || line[0] == 'Q')) {
            printf("Uscita in corso...\r\n");
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
            printf("Nessun comando valido. Riprovare.\r\n");
            continue;
        }

        // --- Lettura sensore ---
        // Leggi temperatura per prima per aggiornare t_fine (necessario per pressione/umidita')
        float t_cached = bme280_read_temperature();

        char tbuf[16], pbuf[20], hbuf[16];

        // Stampa ogni valore su una propria riga, nell'ordine richiesto
        for (int i = 0; i < req_count; ++i) {
            switch (requests[i]) {
                case S_TEMP:
                    dtostrf(t_cached, 6, 2, tbuf);
                    printf("Temperatura: %s C\r\n", tbuf);
                    break;
                case S_PRESS: {
                    float p = bme280_read_pressure();
                    dtostrf(p, 7, 2, pbuf);
                    printf("Pressione: %s hPa\r\n", pbuf);
                    break;
                }
                case S_HUM: {
                    float h = bme280_read_humidity();
                    dtostrf(h, 6, 2, hbuf);
                    printf("Umidita: %s %%\r\n", hbuf);
                    break;
                }
                default:
                    break;
            }
        }
        // Dopo la stampa dei valori, ritorna al prompt per nuova richiesta
    }
}








