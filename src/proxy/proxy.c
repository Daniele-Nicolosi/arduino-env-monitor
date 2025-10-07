#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../../avr_common/uart/uart.h"
#include "../../avr_common/i2c/i2c.h"
#include "../sensors/bme280.h"
#include "../display/oled.h"
#include "../buttons/buttons.h"

/* ------------------------------------------------------------
   Inizializza UART, I2C, sensore BME280, display OLED e pulsanti
------------------------------------------------------------ */
void proxy_init(void) {
    UART_init(UART_MYUBRR);
    i2c_init();
    bme280_init();
    oled_init();
    buttons_init();
}

/* ------------------------------------------------------------
   Mostra il menù principale con la freccia sulla voce selezionata
------------------------------------------------------------ */
static void show_menu(uint8_t sel) {
    oled_clear();
    oled_print_line(0, "SELECT PARAMETER:");
    oled_print_line(2, (sel == 0) ? "--> Temperature" : "    Temperature");
    oled_print_line(3, (sel == 1) ? "--> Pressure"    : "    Pressure");
    oled_print_line(4, (sel == 2) ? "--> Humidity"    : "    Humidity");
    oled_print_line(5, (sel == 3) ? "--> All"         : "    All");
    oled_print_line(6, (sel == 4) ? "--> Exit"        : "    Exit");
}

/* ------------------------------------------------------------
   Mostra i valori letti dai sensori e aggiorna l'OLED
------------------------------------------------------------ */
static void read_and_display(uint8_t mode) {
    enum sensor_t { S_TEMP, S_PRESS, S_HUM };

    float t_cached = bme280_read_temperature();
    char tbuf[32], pbuf[32], hbuf[32];
    strcpy(tbuf, "Temperature:");
    strcpy(pbuf, "Pressure:");
    strcpy(hbuf, "Humidity:");

    // buffer temporanei
    char val[16], msg[64];

    if (mode == 3) {  // modalità "All"
        float t = t_cached;
        float p = bme280_read_pressure();
        float h = bme280_read_humidity();

        dtostrf(t, 6, 2, val);
        snprintf(msg, sizeof(msg), "Temperature: %s C\r\n", val);
        UART_putString(msg);
        snprintf(tbuf, sizeof(tbuf), "Temperature: %s C", val);

        dtostrf(p, 7, 2, val);
        snprintf(msg, sizeof(msg), "Pressure: %s hPa\r\n", val);
        UART_putString(msg);
        snprintf(pbuf, sizeof(pbuf), "Pressure: %s hPa", val);

        dtostrf(h, 6, 2, val);
        snprintf(msg, sizeof(msg), "Humidity: %s %%\r\n", val);
        UART_putString(msg);
        snprintf(hbuf, sizeof(hbuf), "Humidity: %s %%", val);

        oled_show_sensors(tbuf, pbuf, hbuf);
    } 
    else { // modalità singolo parametro
        switch (mode) {
            case S_TEMP: {
                dtostrf(t_cached, 6, 2, val);
                snprintf(msg, sizeof(msg), "Temperature: %s C\r\n", val);
                UART_putString(msg);
                snprintf(tbuf, sizeof(tbuf), "Temperature: %s C", val);
                oled_show_sensor(tbuf, NULL, NULL);
                break;
            }
            case S_PRESS: {
                float p = bme280_read_pressure();
                dtostrf(p, 7, 2, val);
                snprintf(msg, sizeof(msg), "Pressure: %s hPa\r\n", val);
                UART_putString(msg);
                snprintf(pbuf, sizeof(pbuf), "Pressure: %s hPa", val);
                oled_show_sensor(NULL, pbuf, NULL);
                break;
            }
            case S_HUM: {
                float h = bme280_read_humidity();
                dtostrf(h, 6, 2, val);
                snprintf(msg, sizeof(msg), "Humidity: %s %%\r\n", val);
                UART_putString(msg);
                snprintf(hbuf, sizeof(hbuf), "Humidity: %s %%", val);
                oled_show_sensor(NULL, NULL, hbuf);
                break;
            }
            default:
                break;
        }
    }
}

/* ------------------------------------------------------------
   proxy_run()
   Gestisce il menù e la lettura con pulsanti SELECT/CONFIRM
------------------------------------------------------------ */
void proxy_run(void) {
    uint8_t selection = 0;
    uint8_t in_menu = 1;

    show_menu(selection);

    while (1) {
        uint8_t btn = buttons_read();

        if (in_menu) {
            if (btn == 1) { // SELECT → cambia riga
                selection = (selection + 1) % 5;
                show_menu(selection);
            } 
            else if (btn == 2) { // CONFIRM
                if (selection == 4) { // Exit
                    oled_clear();
                    oled_print_line(3, "     GOODBYE! :)");
                    UART_putString("Program terminated.\r\n");
                    _delay_ms(2000);
                    oled_clear();
                    return;
                } 
                else {
                    read_and_display(selection);
                    in_menu = 0;
                }
            }
        } 
        else {
            // dopo la visualizzazione → ritorna al menù
            if (btn == 1 || btn == 2) {
                in_menu = 1;
                show_menu(selection);
            }
        }

        _delay_ms(150);
    }
}












