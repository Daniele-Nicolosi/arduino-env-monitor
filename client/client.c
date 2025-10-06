// client.c - REPL seriale con termios (Linux)
// Usa select() per leggere sia da STDIN che dalla porta seriale.

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200809L

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CRTSCTS
#define CRTSCTS 0
#endif

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

static int serial_open_config(const char *dev, int baud) {
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open serial");
        return -1;
    }

    // prendi i settaggi correnti
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // modalità raw
    cfmakeraw(&tio);

    // 8N1, no flow control
    tio.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
    tio.c_cflag |= (CS8 | CLOCAL | CREAD);

    // baudrate
    speed_t s = map_baud(baud);
    if (!s) {
        fprintf(stderr, "Baud non supportato: %d\n", baud);
        close(fd);
        return -1;
    }
    cfsetispeed(&tio, s);
    cfsetospeed(&tio, s);

    // timeout lettura: non bloccante (VMIN=0), pacchetti ogni 100ms (VTIME=1 -> 0.1s)
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 1;

    // applica subito
    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    // torna blocking per semplicità, usiamo comunque select()
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    // pulisci eventuale buffer residuo
    tcflush(fd, TCIOFLUSH);
    return fd;
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [-d device] [-b baud]\n"
        "  device predefinito: /dev/ttyACM0\n"
        "  baud   predefinito: 19200\n\n"
        "Esempio:\n"
        "  %s -d /dev/ttyACM0 -b 19200\n", prog, prog);
}

int main(int argc, char **argv) {
    const char *dev  = "/dev/ttyACM0";
    int baud = 19200;

    // parse args semplici
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-d") && i+1 < argc) {
            dev = argv[++i];
        } else if (!strcmp(argv[i], "-b") && i+1 < argc) {
            baud = atoi(argv[++i]);
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    int fd = serial_open_config(dev, baud);
    if (fd < 0) return 1;

    printf("Connessione aperta su %s @ %d 8N1.\n", dev, baud);
    printf("Digita comandi (es: \"read temp\") e premi INVIO. Ctrl+C per uscire.\n\n");

    // loop REPL: inoltra STDIN -> seriale, e seriale -> STDOUT
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(fd, &rfds);

        int maxfd = (fd > STDIN_FILENO ? fd : STDIN_FILENO);

        int r = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // Dati dalla seriale -> STDOUT
        if (FD_ISSET(fd, &rfds)) {
            char sbuf[512];
            ssize_t n = read(fd, sbuf, sizeof(sbuf));
            if (n > 0) {
                fwrite(sbuf, 1, (size_t)n, stdout);
                fflush(stdout);
            } else if (n == 0) {
                fprintf(stderr, "\n[Seriale chiusa]\n");
                break;
            } else {
                perror("read(serial)");
                break;
            }
        }

        // Dati da tastiera -> seriale (line-based)
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            char lbuf[512];
            if (!fgets(lbuf, sizeof(lbuf), stdin)) {
                // EOF su stdin
                break;
            }
            // Assicurati che finisca con '\n'
            size_t L = strlen(lbuf);
            if (L == 0 || lbuf[L-1] != '\n') {
                if (L < sizeof(lbuf)-1) {
                    lbuf[L] = '\n';
                    lbuf[L+1] = '\0';
                    L++;
                }
            }
            ssize_t wn = write(fd, lbuf, L);
            if (wn < 0) {
                perror("write(serial)");
                break;
            }
        }
    }

    close(fd);
    return 0;
}