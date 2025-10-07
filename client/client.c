// client.c - Lettura seriale per progetto Arduino I2C Proxy
// Modalità predefinita: lettura log da /dev/ttyACM0 @19200 baud
// Struttura: open → setup → read/write → close

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

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

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    cfmakeraw(&tty);

    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
    tty.c_cflag |= (CS8 | CLOCAL | CREAD);

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;  // timeout 0.5s

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Errore %d da tcsetattr\n", errno);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------
   Imposta modalità bloccante/non bloccante
------------------------------------------------------------ */
static void serial_set_blocking(int fd, int should_block) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Errore %d da tcgetattr\n", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = should_block ? 5 : 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        fprintf(stderr, "Errore %d impostando termios\n", errno);
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
   Usage (help) - marcata come unused per evitare warning
------------------------------------------------------------ */
static void __attribute__((unused)) usage(const char *prog) {
    fprintf(stderr,
        "Uso: %s [device] [baudrate] [mode]\n"
        "  device   predefinito: /dev/ttyACM0\n"
        "  baudrate predefinito: 19200\n"
        "  mode = 1 → lettura log (default)\n"
        "  mode = 0 → scrittura comandi\n\n"
        "Esempi:\n"
        "  %s               (usa /dev/ttyACM0, 19200, lettura)\n"
        "  %s /dev/ttyUSB0 115200 1\n"
        "  %s /dev/ttyACM0 19200 0\n",
        prog, prog, prog, prog);
}

/* ------------------------------------------------------------
   main()
   Default: /dev/ttyACM0 @19200 baud, modalità log
------------------------------------------------------------ */
int main(int argc, char **argv) {
    const char *device = "/dev/ttyACM0";
    int baud = 19200;
    int mode = 1; // lettura log per default

    if (argc > 1) device = argv[1];
    if (argc > 2) baud = atoi(argv[2]);
    if (argc > 3) mode = atoi(argv[3]);

    int fd = serial_open(device);
    if (fd < 0) return 1;

    if (serial_set_interface_attribs(fd, baud) < 0) {
        close(fd);
        return 1;
    }

    serial_set_blocking(fd, 1);

    printf("==============================================\n");
    printf("   🔌 Arduino I2C Proxy - Serial Monitor\n");
    printf("==============================================\n");
    printf(" Dispositivo: %s\n", device);
    printf(" Baudrate:    %d\n", baud);
    printf(" Modalità:    %s\n", mode ? "Lettura log (default)" : "Scrittura comandi");
    printf("----------------------------------------------\n");
    printf(" Premi Ctrl+C per uscire.\n\n");

    char buf[512];
    while (1) {
        memset(buf, 0, sizeof(buf));

        if (mode) {
            // Lettura log dal proxy
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                printf("%s", buf);
                fflush(stdout);
            }
        } else {
            // Scrittura (eventuale estensione futura)
            if (!fgets(buf, sizeof(buf), stdin)) break;
            size_t len = strlen(buf);
            if (len > 0 && buf[len - 1] != '\n') {
                buf[len] = '\n';
                buf[len + 1] = '\0';
                len++;
            }
            ssize_t wn = write(fd, buf, len);
            if (wn < 0) {
                perror("write");
            }
        }
    }

    close(fd);
    printf("\nConnessione chiusa.\n");
    return 0;
}


