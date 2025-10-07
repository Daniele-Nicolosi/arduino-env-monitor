#include <avr/io.h>
#include <util/delay.h>
#include "proxy/proxy.h"
#include "display/oled.h"

int main(void) {
    proxy_init();

    oled_clear();
    oled_print_line(3, "       WELCOME!");
    _delay_ms(2000);

    proxy_run();

    return 0;
}



  
 











































































