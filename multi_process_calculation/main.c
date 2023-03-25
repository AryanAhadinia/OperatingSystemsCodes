#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>

extern long fun(long a, long b);

char *FILE_PATH = "./Practical_Q_1numbers.txt";
char *FIFO_PIPE = "FIFO_PIPE";

long EOS = -1;

int main() {
    int pipe_descriptors[8][2]; // for 8 subprocess
    for (int i = 0; i < 8; i++) {
        int status = pipe(pipe_descriptors[i]);
        if (status == -1) {
            printf("Cannot initiate pipe.\n");
            return -1;
        }
    }
    int pid = -1;
    int serial_id = 0;
    while (pid != 0 && serial_id < 8) {
        pid = fork();
        if (pid != 0) { // only parent increase serial
            serial_id++;
        }
    }
    if (pid != 0) { // parent read file
        long file_numbers[400000];
        long file_numbers_len = 0;
        FILE *file_pointer;
        file_pointer = fopen(FILE_PATH, "r");
        if (NULL == file_pointer) {
            printf("Cannot open file.\n");
            return -1;
        }
        char buffer[32];
        long num_buffer;
        while (fscanf(file_pointer, "%s\n", buffer) == 1) {
            sscanf(buffer, "%ld", &num_buffer);
            file_numbers[file_numbers_len++] = num_buffer;
        }
        fclose(file_pointer);
        for (int i = 0; i < file_numbers_len; ++i) {
            write(pipe_descriptors[i % 8][1], &file_numbers[i], sizeof(file_numbers[i]));
        }
        for (int i = 0; i < 8; ++i) {
            write(pipe_descriptors[i][1], &EOS, sizeof(EOS));
        }
        long result[8];
        mkfifo(FIFO_PIPE, S_IFIFO|0640);
        int fifo_pipe_descriptor = open(FIFO_PIPE, O_RDWR);
        for (int i = 0; i < 8; ++i) {
            read(fifo_pipe_descriptor, &num_buffer, sizeof(long));
            result[i] = num_buffer;
        }
        close(fifo_pipe_descriptor);
        long acc = result[0];
        for (int i = 1; i < 8; ++i) {
            acc = fun(acc, result[i]);
        }
        printf("Result of running `fun` associatively on data is `%ld`.\n", acc);
    } else {
        long buffer;
        long subprocess_numbers[80000];
        long subprocess_numbers_len = 0;
        do {
            read(pipe_descriptors[serial_id][0], &buffer, sizeof(buffer));
            subprocess_numbers[subprocess_numbers_len++] = buffer;
        } while (buffer != -1);
        subprocess_numbers_len--;
        long acc = subprocess_numbers[0];
        for (int i = 1; i < subprocess_numbers_len; ++i) {
            acc = fun(acc, subprocess_numbers[i]);
        }
        mkfifo(FIFO_PIPE, S_IFIFO|0640);
        int fifo_pipe_descriptor = open(FIFO_PIPE, O_RDWR);
        write(fifo_pipe_descriptor, &acc, sizeof(long));
        close(fifo_pipe_descriptor);
    }
    return 0;
}
