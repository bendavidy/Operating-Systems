#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int input_fd, output_fd;
char* buffer;

void close_all_and_teminate() {
    close(input_fd);
    close(output_fd);
    free(buffer);
    exit(1);
}

int main(int argc, char* argv[]) {
    int n, read_chars, write_chars;

    if (argc != 4) {
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(1);
    }
    
    if (sscanf(argv[3], "%d", &n) < 1 || n < 1) {
        fprintf(stderr, "Incorrect argument for 'n'\n");
        close(input_fd);
        close(output_fd);
        exit(1);
    }

    input_fd = open(argv[1], O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    if (input_fd == -1) {
        perror("open");
        exit(1);
    }

    output_fd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IXUSR);
    if (output_fd == -1) {
        perror("open");
        close(input_fd);
        exit(1);
    }
    

    buffer = (char*)malloc(n);
    
    while (0 < (read_chars = read(input_fd, buffer, n))){
        write_chars = write(output_fd, buffer, read_chars);
        if (write_chars == -1){
            perror("write");
            close_all_and_teminate();
        }
        if (write_chars < read_chars){
            fprintf(stderr, "Written less characters than what was read\n");
            close_all_and_teminate();
        }
    }
    if (read_chars == -1){
        perror("read");
        close_all_and_teminate();
    }

    close(input_fd);
    close(output_fd);
    free(buffer);

    return 0; /* successful return */
}
