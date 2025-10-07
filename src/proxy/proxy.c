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

// --- Configurazione globale ---
static uint16_t sampling_ms = 1000;
static uint8_t log_enabled = 1;
static temp_unit_t temp_unit = UNIT_C;
static press_unit_t press_unit = UNIT_PA;

static float last_temp = 0.0f;
static float last_press = 0.0f;
static float last_hum = 0.0f;

// --- Utility: converte stringa in minuscolo ---
static void str_to_lower(char *s) {
    for (; *s; ++s)
        *s = (*s >= 'A' && *s <= 'Z') ? *s + 32 : *s;
}

// --- Formattazione valori ---
static void format_temp(char *out, size_t n) {
    float t = last_temp;
    const char *unit = "C";
    if (temp_unit == UNIT_K) { t = t + 273.15f; unit = "K"; }
    else if (temp_unit == UNIT_F) { t = (t * 9.0f / 5.0f) + 32.0f; unit = "F"; }

    char buf[16];
    dtostrf(t, 6, 2, buf);
    snprintf(out, n, "Temperature: %s %s", buf, unit);
}

static void format_press(char *out, size_t n) {
    float p = last_press;
    if (press_unit == UNIT_BAR) {
        p = p / 1000.0f;  // da hPa a bar
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

// --- Configurazione iniziale da terminale ---
static void proxy_configure(void) {
    char buf[32];
    UART_putString("\r\n=== CONFIGURAZIONE PROXY ===\r\n");
    UART_putString("Seleziona sampling rate (ms):\r\n");
    UART_putString("1) 250\r\n2) 500\r\n3) 1000\r\n4) 2000\r\n5) 5000\r\n> ");

    while (1) {
        UART_getString(buf, sizeof(buf));
        int opt = atoi(buf);
        if (opt >= 1 && opt <= 5) {
            switch (opt) {
                case 1: sampling_ms = 250; break;
                case 2: sampling_ms = 500; break;
                case 3: sampling_ms = 1000; break;
                case 4: sampling_ms = 2000; break;
                case 5: sampling_ms = 5000; break;
            }
            break;
        }
        UART_putString("Valore non valido. Riprova: ");
    }

    UART_putString("Unita' temperatura (C/K/F): ");
    while (1) {
        UART_getString(buf, sizeof(buf));
        str_to_lower(buf);
        if (!strcmp(buf, "c")) { temp_unit = UNIT_C; break; }
        if (!strcmp(buf, "k")) { temp_unit = UNIT_K; break; }
        if (!strcmp(buf, "f")) { temp_unit = UNIT_F; break; }
        UART_putString("Valore non valido (C/K/F): ");
    }

    UART_putString("Unita' pressione (Pa/bar): ");
    while (1) {
        UART_getString(buf, sizeof(buf));
        str_to_lower(buf);
        if (!strcmp(buf, "pa")) { press_unit = UNIT_PA; break; }
        if (!strcmp(buf, "bar")) { press_unit = UNIT_BAR; break; }
        UART_putString("Valore non valido (Pa/bar): ");
    }

    UART_putString("Abilitare log? (on/off): ");
    while (1) {
        UART_getString(buf, sizeof(buf));
        str_to_lower(buf);
        if (!strcmp(buf, "on")) { log_enabled = 1; break; }
        if (!strcmp(buf, "off")) { log_enabled = 0; break; }
        UART_putString("Valore non valido (on/off): ");
    }

    UART_putString("----------------------------\r\n");
    char conf[128];
    snprintf(conf, sizeof(conf),
             "Sampling: %u ms | Temp: %s | Press: %s | Log: %s\r\n",
             sampling_ms,
             (temp_unit == UNIT_C ? "C" : temp_unit == UNIT_K ? "K" : "F"),
             (press_unit == UNIT_BAR ? "bar" : "hPa"),
             (log_enabled ? "ON" : "OFF"));
    UART_putString(conf);
    UART_putString("Inizializzazione display...\r\n\r\n");
}

// --- Inizializzazione generale ---
void proxy_init(void) {
    UART_init(UART_MYUBRR);
    i2c_init();
    bme280_init();

    proxy_configure();

    oled_init();
    buttons_init();

    // prime letture
    last_temp = bme280_read_temperature();
    last_press = bme280_read_pressure();
    last_hum = bme280_read_humidity();
}

// --- Menu display ---
static void show_menu(uint8_t sel) {
    oled_clear();
    oled_print_line(0, "SELECT PARAMETER:");
    oled_print_line(2, (sel == 0) ? "--> Temperature" : "    Temperature");
    oled_print_line(3, (sel == 1) ? "--> Pressure"    : "    Pressure");
    oled_print_line(4, (sel == 2) ? "--> Humidity"    : "    Humidity");
    oled_print_line(5, (sel == 3) ? "--> All"         : "    All");
    oled_print_line(6, (sel == 4) ? "--> Exit"        : "    Exit");
}

// --- Visualizzazione valori e log ---
static void display_and_log(uint8_t sel) {
    char tbuf[32], pbuf[32], hbuf[32];
    format_temp(tbuf, sizeof(tbuf));
    format_press(pbuf, sizeof(pbuf));
    format_hum(hbuf, sizeof(hbuf));

    if (sel == 3) {
        oled_show_sensors(tbuf, pbuf, hbuf);
        if (log_enabled) {
            UART_putString(tbuf); UART_putString("\r\n");
            UART_putString(pbuf); UART_putString("\r\n");
            UART_putString(hbuf); UART_putString("\r\n");
        }
    } else {
        switch (sel) {
            case 0:
                oled_show_sensor(tbuf, NULL, NULL);
                if (log_enabled) UART_putString(tbuf);
                break;
            case 1:
                oled_show_sensor(NULL, pbuf, NULL);
                if (log_enabled) UART_putString(pbuf);
                break;
            case 2:
                oled_show_sensor(NULL, NULL, hbuf);
                if (log_enabled) UART_putString(hbuf);
                break;
        }
        if (log_enabled) UART_putString("\r\n");
    }
}

// --- Ciclo principale ---
void proxy_run(void) {
    uint8_t sel = 0;
    uint8_t in_menu = 1;
    uint16_t elapsed = 0;

    show_menu(sel);

    while (1) {
        if (elapsed >= sampling_ms) {
            last_temp = bme280_read_temperature();
            last_press = bme280_read_pressure();
            last_hum = bme280_read_humidity();
            elapsed = 0;
        }

        uint8_t btn = buttons_read();

        if (in_menu) {
            if (btn == 1) {
                sel = (sel + 1) % 5;
                show_menu(sel);
            } else if (btn == 2) {
                if (sel == 4) {
                    UART_putString("#EXIT#\r\n");
                    oled_clear();
                    oled_print_line(3, "     GOODBYE! :)");
                    _delay_ms(2000);
                    oled_clear();
                    return;
                } else {
                    display_and_log(sel);
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
        elapsed += 10;
    }
}













