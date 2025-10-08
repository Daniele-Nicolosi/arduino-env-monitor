#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

int serial_set_interface_attribs(int fd, int speed) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    speed_t baud;
    switch (speed) {
        case 9600: baud = B9600; break;
        case 19200: baud = B19200; break;
        case 57600: baud = B57600; break;
        case 115200: baud = B115200; break;
        default:
            fprintf(stderr, "Unsupported baudrate %d\n", speed);
            return -1;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);
    cfmakeraw(&tty);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

int serial_open(const char* device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    return fd;
}

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

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; fds[0].events = POLLIN;
    fds[1].fd = fd;           fds[1].events = POLLIN;

    while (1) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) break;

        // input da tastiera → seriale
        if (fds[0].revents & POLLIN) {
            char line[1024];
            if (!fgets(line, sizeof(line), stdin)) break;
            size_t len = strlen(line);
            if (len && line[len-1] == '\n') line[len-1] = '\n';
            ssize_t written = write(fd, line, len);
            if (written < 0) perror("write");
        }

        // input dalla seriale → terminale
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

    close(fd);
    return 0;
}








