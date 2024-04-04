#include <user/user.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"

directory_t dir_entry_buffer;

void ls() {
    uint32_t dir = open(cwd(), 0);
    if (dir == (uint32_t) -1) {
        printf("failed to open directory\n");
        return;
    }
    do {
        if (!read(dir, (char *) &dir_entry_buffer, 512)) {
            printf("failed to read directory\n");
            break;
        }
        printf("%s\n", dir_entry_buffer.name);
    } while (*dir_entry_buffer.name);
    close(dir);
}
