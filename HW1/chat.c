#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

static int cleanup(int fd_in, int fd_out, unsigned char *buffer)
{
    int ok = 0;

    if (fd_in != -1) {
        if (close(fd_in) == -1) {
            perror("close");
            ok = -1;
        }
    }

    if (fd_out != -1) {
        if (close(fd_out) == -1) {
            perror("close");
            ok = -1;
        }
    }

    free(buffer);
    return ok;
}

int main(int argc, char *argv[])
{
    int fd_in = -1, fd_out = -1;
    unsigned char *buffer = NULL;
    char *endptr = NULL;
    long n_long;
    size_t n;
    ssize_t bytes_read, bytes_written;
    ssize_t total_written;

    if (argc != 4) {
        fprintf(stderr, "not enough arguments\n");
        exit(1);
    }

    n_long = strtol(argv[3], &endptr, 10);

    /*
    Invalid cases:
    - non-numeric input: "s"
    - negative number: "-3"
    - floating point: "4.5"
    - zero: "0"
    */
    if (*endptr != '\0' || n_long <= 0) {
        fprintf(stderr, "invalid buffer size\n");
        exit(1);
    }

    n = (size_t)n_long;
    buffer = (unsigned char *)malloc(n);
    if (buffer == NULL) {
        perror("malloc");
        exit(1);
    }

    fd_in = open(argv[1], O_RDONLY);
    if (fd_in == -1) {
        perror("open");
        cleanup(fd_in, fd_out, buffer);
        exit(1);
    }

    fd_out = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd_out == -1) {
        perror("open");
        cleanup(fd_in, fd_out, buffer);
        exit(1);
    }

    while ((bytes_read = read(fd_in, buffer, n)) > 0) {
        total_written = 0;

        while (total_written < bytes_read) {
            bytes_written = write(fd_out,
                                  buffer + total_written,
                                  (size_t)(bytes_read - total_written));

            if (bytes_written == -1) {
                perror("write");
                cleanup(fd_in, fd_out, buffer);
                exit(1);
            }

            total_written += bytes_written;
        }
    }

    if (bytes_read == -1) {
        perror("read");
        cleanup(fd_in, fd_out, buffer);
        exit(1);
    }

    if (cleanup(fd_in, fd_out, buffer) == -1) {
        exit(1);
    }

    return 0;
}