// client.c - REPL full-duplex per progetto Arduino I2C Proxy
// Default: /dev/ttyACM0 @19200, inoltro tastiera->seriale e stampa log seriale->stdout
// Struttura: open → setup (termios) → loop select() (read/write) → close

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>

#ifndef CRTSCTS
#define CRTSCTS 0
#endif

/* ------------------------------------------------------------
   Mappa il baudrate numerico al valore termios corrispondente
------------------------------------------------------------ */
static speed_t map_baud(int baud) {
    switch (baud) {
        case 1200:   return B1200;
        case 2400:   return B2400;
        case 4800:   return B4800;
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return 0;
    }
}

/* ------------------------------------------------------------
   Configura la seriale (8N1, no flow control)
------------------------------------------------------------ */
static int serial_set_interface_attribs(int fd, int baudrate) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Errore %d da tcgetattr\n", errno);
        return -1;
    }

    speed_t speed = map_baud(baudrate);
    if (!speed) {
        fprintf(stderr, "Baudrate non supportato: %d\n", baudrate);
        return -1;
    }

    cfmakeraw(&tty);                // modalità RAW
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    // 8N1, nessun flow control
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
    tty.c_cflag |= (CS8 | CLOCAL | CREAD);

    // Letture "reattive": non bloccare in read() (VMIN=0), timeout 0.1s (VTIME=1)
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Errore %d da tcsetattr\n", errno);
        return -1;
    }

    // (opzionale) torna blocking a livello di file descriptor,
    // ma con VMIN/VTIME e select() siamo già a posto.
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1) fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    tcflush(fd, TCIOFLUSH);         // pulizia buffer I/O
    return 0;
}

/* ------------------------------------------------------------
   Apre il dispositivo seriale
------------------------------------------------------------ */
static int serial_open(const char *name) {
    int fd = open(name, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "Errore %d aprendo %s\n", errno, name);
        return -1;
    }
    return fd;
}

/* ------------------------------------------------------------
   main()
   Default: /dev/ttyACM0 @19200 baud, modalità REPL (duplex)
------------------------------------------------------------ */
int main(int argc, char **argv) {
    const char *device = "/dev/ttyACM0";
    int baud = 19200;

    if (argc > 1) device = argv[1];
    if (argc > 2) baud    = atoi(argv[2]);

    int fd = serial_open(device);
    if (fd < 0) return 1;

    if (serial_set_interface_attribs(fd, baud) < 0) {
        close(fd);
        return 1;
    }

    printf("==============================================\n");
    printf("   🔌 Arduino I2C Proxy - Terminale seriale\n");
    printf("==============================================\n");
    printf(" Dispositivo: %s\n", device);
    printf(" Baudrate:    %d\n", baud);
    printf("----------------------------------------------\n");
    printf(" • Digita risposte/valori e premi INVIO.\n");
    printf(" • Il client invierà sempre <CR> (carriage return) alla scheda.\n");
    printf(" • Uscita automatica quando la scheda invia #EXIT#.\n");
    printf(" • In alternativa, Ctrl+C per terminare.\n\n");

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);               // seriale
        FD_SET(STDIN_FILENO, &rfds);     // tastiera

        int maxfd = (fd > STDIN_FILENO ? fd : STDIN_FILENO);
        int r = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // --- Dalla seriale → stdout ---
        if (FD_ISSET(fd, &rfds)) {
            char sbuf[512];
            ssize_t n = read(fd, sbuf, sizeof(sbuf));
            if (n > 0) {
                fwrite(sbuf, 1, (size_t)n, stdout);
                fflush(stdout);

                // chiusura remota
                if (memmem(sbuf, (size_t)n, "#EXIT#", 6) != NULL) {
                    printf("\nChiusura in corso...\n");
                    break;
                }
            }
            // n==0: nessun dato (timeout VTIME), ignora
            else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("read(serial)");
                break;
            }
        }

        // --- Da tastiera → seriale ---
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            char lbuf[512];
            if (!fgets(lbuf, sizeof(lbuf), stdin)) {
                // EOF su stdin
                break;
            }

            // rimuovi \n e/o \r finali
            size_t L = strlen(lbuf);
            while (L > 0 && (lbuf[L-1] == '\n' || lbuf[L-1] == '\r')) {
                lbuf[--L] = '\0';
            }

            // aggiungi sempre CR per soddisfare UART_getString()
            if (L < sizeof(lbuf) - 1) {
                lbuf[L++] = '\r';
                lbuf[L]   = '\0';
            }

            ssize_t wn = write(fd, lbuf, L);
            if (wn < 0) {
                perror("write(serial)");
                break;
            }
        }
    }

    close(fd);
    printf("\nConnessione chiusa.\n");
    return 0;
}




