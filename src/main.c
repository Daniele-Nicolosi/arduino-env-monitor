#include "proxy/proxy.h"

/* ------------------------------------------------------------
   main()
   Punto d’ingresso del programma.
   - Inizializza il proxy (UART, I2C, sensore, display, pulsanti)
   - Avvia il menù interattivo controllato da pulsanti
------------------------------------------------------------ */
int main(void) {

    /* --------------------------------------------------------
       Inizializzazione del sistema e configurazione utente
    -------------------------------------------------------- */
    proxy_init();

    /* --------------------------------------------------------
       Avvio del ciclo principale (menù interattivo)
    -------------------------------------------------------- */
    proxy_run();

    return 0;
}




  
 











































































