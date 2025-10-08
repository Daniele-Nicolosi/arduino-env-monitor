#include <poll.h> 

#include "client.h"

/* ------------------------------------------------------------
   serial_set_interface_attribs()
   Configura i parametri della porta seriale.
   - Imposta baudrate (9600, 19200, 57600, 115200)
   - Modalità raw (nessuna interpretazione dei byte)
   - 8 bit, nessuna parità, 1 stop bit
------------------------------------------------------------ */
int serial_set_interface_attribs(int fd, int speed) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    speed_t baud;
    switch (speed) {
        case 9600:   baud = B9600; break;
        case 19200:  baud = B19200; break;
        case 57600:  baud = B57600; break;
        case 115200: baud = B115200; break;
        default:
            fprintf(stderr, "Unsupported baudrate %d\n", speed);
            return -1;
    }

    // Imposta velocità di trasmissione e ricezione
    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    // Imposta modalità "raw" (nessuna elaborazione del driver)
    cfmakeraw(&tty);

    // Configura 8N1 (8 bit, nessuna parità, 1 stop)
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;

    // Lettura non bloccante (timeout 0.1 s)
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------
   serial_open()
   Apre il dispositivo seriale in modalità lettura/scrittura.
   Flag:
   - O_RDWR  : lettura e scrittura
   - O_NOCTTY: non diventa terminale controllante
   - O_SYNC  : scrittura sincrona (flush immediato)
------------------------------------------------------------ */
int serial_open(const char* device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    return fd;
}

/* ------------------------------------------------------------
   main()
   Programma principale del client seriale.
   Permette di comunicare con il proxy Arduino tramite terminale.
   Funzioni principali:
   - Connessione alla porta seriale indicata
   - Lettura e scrittura simultanee (con poll)
------------------------------------------------------------ */
int main(int argc, const char** argv) {
    if (argc < 3) {
        printf("Usage: client <serial_device> <baudrate>\n");
        return 1;
    }

    const char* device = argv[1];
    int baudrate = atoi(argv[2]);

    int fd = serial_open(device);
    serial_set_interface_attribs(fd, baudrate);

    printf("Connected to %s @ %d baud\n", device, baudrate);
    printf("Type and press Enter to send. Ctrl+C to exit.\n");

    /* --------------------------------------------------------
       Configura polling su due file descriptor:
       - fds[0]: input da tastiera (STDIN)
       - fds[1]: input dalla seriale (fd)
       Il polling consente di gestire entrambi senza blocchi.
    -------------------------------------------------------- */
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; fds[0].events = POLLIN;
    fds[1].fd = fd;           fds[1].events = POLLIN;

    while (1) {
        int ret = poll(fds, 2, -1); // attende eventi da tastiera o seriale
        if (ret < 0) break;

        /* ----------------------------------------------------
           Input da tastiera → invio sulla seriale
           Legge una riga con fgets(), la invia con '\n' finale.
        ---------------------------------------------------- */
        if (fds[0].revents & POLLIN) {
            char line[1024];
            if (!fgets(line, sizeof(line), stdin)) break;
            size_t len = strlen(line);
            if (len && line[len-1] == '\n') line[len-1] = '\n';
            ssize_t written = write(fd, line, len);
            if (written < 0) perror("write");
        }

        /* ----------------------------------------------------
           Input dalla seriale → stampa sul terminale
           Tutto ciò che Arduino invia viene visualizzato
           direttamente, senza modifiche.
        ---------------------------------------------------- */
        if (fds[1].revents & POLLIN) {
            char buf[256];
            int n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                ssize_t written = write(STDOUT_FILENO, buf, n);
                if (written < 0) perror("write"); 
                fflush(stdout);
            }
        }
    }

    /* --------------------------------------------------------
       Chiusura della connessione seriale
    -------------------------------------------------------- */
    close(fd);
    return 0;
}









