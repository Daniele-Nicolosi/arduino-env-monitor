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
#include "proxy.h"

/* ------------------------------------------------------------
   Configurazione globale
------------------------------------------------------------ */
static uint16_t sampling_ms = 1000;
static uint8_t  log_enabled = 1;
static temp_unit_t  temp_unit  = UNIT_C;
static press_unit_t press_unit = UNIT_PA;

static float last_temp  = 0.0f;
static float last_press = 0.0f;
static float last_hum   = 0.0f;

/* ------------------------------------------------------------
   Converte una stringa in minuscolo
------------------------------------------------------------ */
static void str_to_lower(char *s) {
    while (*s) {
        if (*s >= 'A' && *s <= 'Z') *s += ('a' - 'A');
        s++;
    }
}

/* ------------------------------------------------------------
   Formatta i valori letti dai sensori
------------------------------------------------------------ */
static void format_temp(char *out, size_t n) {
    float t = last_temp;
    const char *unit = "C";
    if (temp_unit == UNIT_K) { t += 273.15f; unit = "K"; }
    else if (temp_unit == UNIT_F) { t = t * 9.0f / 5.0f + 32.0f; unit = "F"; }

    char buf[16];
    dtostrf(t, 6, 2, buf);
    snprintf(out, n, "Temperature: %s %s", buf, unit);
}

static void format_press(char *out, size_t n) {
    float p = last_press;
    if (press_unit == UNIT_BAR) {
        p /= 1000.0f;
        char buf[16];
        dtostrf(p, 7, 3, buf);
        snprintf(out, n, "Pressure: %s bar", buf);
    } else {
        char buf[16];
        dtostrf(p, 7, 2, buf);
        snprintf(out, n, "Pressure: %s hPa", buf);
    }
}

static void format_hum(char *out, size_t n) {
    char buf[16];
    dtostrf(last_hum, 6, 2, buf);
    snprintf(out, n, "Humidity: %s %%", buf);
}

/* ------------------------------------------------------------
   Mostra un’introduzione del progetto sul terminale
------------------------------------------------------------ */
static void PROXY_intro(void) {
    UART_putString("\r\n\r\n=============================== PROJECT OVERVIEW ===============================\r\n");
    UART_putString("This project implements an Arduino-based Environmental Monitor featuring:\r\n");
    UART_putString("- A BME280 sensor for temperature, pressure, and humidity measurements\r\n");
    UART_putString("- An OLED display for real-time data visualization\r\n");
    UART_putString("- Two buttons for user interaction:\r\n");
    UART_putString("    * LEFT  button: scroll through menu\r\n");
    UART_putString("    * RIGHT button: confirm selection\r\n");
    UART_putString("\r\nTo exit, select \"Exit\" from the display menu.\r\n");
    UART_putString("================================================================================\r\n\r\n");
}

/* ------------------------------------------------------------
   PROXY_configure()
   Gestisce la configurazione 
------------------------------------------------------------ */
static void PROXY_configure(void) {
    char buf[32];
    UART_putString("\r\n\r\n================================ CONFIGURATION =================================\r\n");
    UART_putString("Select sampling rate (1-4):\r\n");
    UART_putString("1) 125 ms\r\n2) 250 ms\r\n3) 500 ms\r\n4) 1000 ms\r\n> ");

    while (1) {
        UART_getString(buf, sizeof(buf));
        int opt = atoi(buf);
        if (opt >= 1 && opt <= 5) {
            switch (opt) {
                case 1: sampling_ms = 125; break;
                case 2: sampling_ms = 250; break;
                case 3: sampling_ms = 500; break;
                case 4: sampling_ms = 1000; break;
            }
            BME280_set_sampling(sampling_ms);
            break;
        }
        UART_putString("Invalid value. Enter a number from 1 to 5: ");
    }

    UART_putString("Temperature unit (C/K/F): ");
    while (1) {
        UART_getString(buf, sizeof(buf));
        str_to_lower(buf);
        if (!strcmp(buf, "c")) { temp_unit = UNIT_C; break; }
        if (!strcmp(buf, "k")) { temp_unit = UNIT_K; break; }
        if (!strcmp(buf, "f")) { temp_unit = UNIT_F; break; }
        UART_putString("Invalid value (C/K/F): ");
    }

    UART_putString("Pressure unit (Pa/bar): ");
    while (1) {
        UART_getString(buf, sizeof(buf));
        str_to_lower(buf);
        if (!strcmp(buf, "pa"))  { press_unit = UNIT_PA;  break; }
        if (!strcmp(buf, "bar")) { press_unit = UNIT_BAR; break; }
        UART_putString("Invalid value (Pa/bar): ");
    }

    UART_putString("Enable terminal log? (on/off): ");
    while (1) {
        UART_getString(buf, sizeof(buf));
        str_to_lower(buf);
        if (!strcmp(buf, "on"))  { log_enabled = 1; break; }
        if (!strcmp(buf, "off")) { log_enabled = 0; break; }
        UART_putString("Invalid value (on/off): ");
    }

    UART_putString("================================================================================\r\n");
    char conf[128];
    snprintf(conf, sizeof(conf),
             "Sampling: %u ms | Temp: %s | Press: %s | Log: %s\r\n",
             sampling_ms,
             (temp_unit == UNIT_C ? "C" : temp_unit == UNIT_K ? "K" : "F"),
             (press_unit == UNIT_BAR ? "bar" : "hPa"),
             (log_enabled ? "ON" : "OFF"));
    UART_putString(conf);
    UART_putString("Configuration complete!\r\n");

    PROXY_intro();
}

/* ------------------------------------------------------------
   PROXY_init()
   - Inizializza UART, I2C, BME280, OLED e pulsanti 
   - Include la fase di configurazione utente
   - Mostra messaggio di benvenuto sul display
------------------------------------------------------------ */
void PROXY_init(void) {
    UART_init(UART_MYUBRR);
    I2C_init();
    BME280_init();

    PROXY_configure();  

    OLED_init();
    BUTTONS_init();

    OLED_clear();
    OLED_print_line(3, "       WELCOME!");
    _delay_ms(2000);
}

/* ------------------------------------------------------------
   show_menu()
   Mostra il menù principale sul display
------------------------------------------------------------ */
static void show_menu(uint8_t sel) {
    OLED_clear();
    OLED_print_line(0, "SELECT PARAMETER:");
    OLED_print_line(2, (sel == 0) ? "--> Temperature" : "    Temperature");
    OLED_print_line(3, (sel == 1) ? "--> Pressure"    : "    Pressure");
    OLED_print_line(4, (sel == 2) ? "--> Humidity"    : "    Humidity");
    OLED_print_line(5, (sel == 3) ? "--> All"         : "    All");
    OLED_print_line(6, (sel == 4) ? "--> Exit"        : "    Exit");
}

/* ------------------------------------------------------------
   show_value()
   Mostra i valori e li invia sulla seriale
------------------------------------------------------------ */
static void show_value(uint8_t sel) {
    char tbuf[32], pbuf[32], hbuf[32];
    format_temp(tbuf, sizeof(tbuf));
    format_press(pbuf, sizeof(pbuf));
    format_hum(hbuf, sizeof(hbuf));

    if (sel == 3) {
        OLED_show_sensors(tbuf, pbuf, hbuf);
        if (log_enabled) {
            UART_putString(tbuf); UART_putString("\r\n");
            UART_putString(pbuf); UART_putString("\r\n");
            UART_putString(hbuf); UART_putString("\r\n");
        }
    } else {
        const char *msg = NULL;
        if (sel == 0) msg = tbuf;
        else if (sel == 1) msg = pbuf;
        else if (sel == 2) msg = hbuf;
        OLED_show_sensor((sel == 0) ? tbuf : NULL,
                         (sel == 1) ? pbuf : NULL,
                         (sel == 2) ? hbuf : NULL);
        if (log_enabled && msg) {
            UART_putString(msg);
            UART_putString("\r\n");
        }
    }
}

/* ------------------------------------------------------------
   PROXY_run()
   Ciclo principale con gestione menù e pulsanti
------------------------------------------------------------ */
void PROXY_run(void) {
    uint8_t sel = 0;
    uint8_t in_menu = 1;

    show_menu(sel);

    while (1) {
        
        last_temp  = BME280_read_temperature();
        last_press = BME280_read_pressure();
        last_hum   = BME280_read_humidity();

        uint8_t btn = BUTTONS_read();

        if (in_menu) {
            if (btn == 1) {
                sel = (sel + 1) % 5;
                show_menu(sel);
            } else if (btn == 2) {
                if (sel == 4) {
                    UART_putString("================================================================================\r\n\r\n");
                    UART_putString("\r\nExiting...\r\n");
                    OLED_clear();
                    OLED_print_line(3, "     GOODBYE! :)");
                    _delay_ms(2000);
                    OLED_clear();
                    UART_putString("Exit complete. Goodbye! :)\r\n");
                    _delay_ms(100);
                    return;
                } else {
                    show_value(sel);
                    in_menu = 0;
                }
            }
        } else {
            if (btn == 1 || btn == 2) {
                in_menu = 1;
                show_menu(sel);
            }
        }

        _delay_ms(10);
    }
}















