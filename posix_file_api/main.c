#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int file_desc_read = open("/Users/aryanahadinia/Desktop/OS_HW2/file_to_read.txt", O_RDONLY);
    if (file_desc_read < 0) {
        char *err = "Cannot open read file.";
        write(1, err, strlen(err));
        return -1;
    }
    off_t size = lseek(file_desc_read, 0, SEEK_END);
    off_t status = lseek(file_desc_read, 0, SEEK_SET);
    if (size == -1 || status == -1) {
        char *err = "Cannot seek on file.";
        write(1, err, strlen(err));
        return -1;
    }
    char *buffer = malloc(size + 1);
    if (buffer == NULL) {
        char *err = "Cannot allocate buffer.";
        write(1, err, strlen(err));
        return -1;
    }
    read(file_desc_read, buffer, size);
    int file_desc_write = open("/Users/aryanahadinia/Desktop/OS_HW2/file_to_write.txt", O_WRONLY | O_CREAT);
    if (file_desc_write < 0) {
        char *err = "Cannot open write file.";
        write(1, err, strlen(err));
        return -1;
    }
    int fork_stat = fork();
    if (fork_stat < 0) {
        char *err = "Cannot fork.";
        write(1, err, strlen(err));
        return -1;
    }
    wait(NULL);
    if (fork_stat == 0) {
        // Child
        char *reversed_buffer = malloc(size + 2);
        for (int i = 0; i < size; ++i) {
            reversed_buffer[i] = buffer[size - i - 1];
        }
        reversed_buffer[size] = '\n';
        reversed_buffer[size + 1] = 0;
        if (write(file_desc_write, reversed_buffer, size + 1) == -1) {
            char *err = "Cannot write in child process.";
            write(1, err, strlen(err));
            return -1;
        }
    } else {
        // Parent
        if (write(file_desc_write, buffer, size) == -1) {
            char *err = "Cannot write in parent process.";
            write(1, err, strlen(err));
            return -1;
        }
    }
    return 0;
}
