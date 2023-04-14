#include <user/user.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"

void cp(char *source, char *destination) {
    if (!source || !destination) {
        printf("usage: cp <source> <destination>\n");
        return;
    }

    uint32_t source_file = open(source, MODE_READ);
    uint32_t destination_file = open(destination, MODE_CREATE | MODE_WRITE);

    if (!source_file) {
        printf("failed to open source file: %s\n", source);
        return;
    }
    if (!destination_file) {
        printf("failed to open destination file: %s\n", destination);
        return;
    }

    uint32_t bytes_read;
    uint32_t bytes_read_total = 0;
    char buffer[512];
    do {
        bytes_read = read(source_file, buffer, 512);
        write(destination_file, buffer, bytes_read);
        bytes_read_total += bytes_read;
        printf("bytes copied: %u\r", bytes_read_total);
    } while (bytes_read);
    putchar('\n');

    close(source_file);
    close(destination_file);
}
