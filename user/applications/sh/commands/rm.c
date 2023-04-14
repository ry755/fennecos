#include <user/user.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"

void rm(char *path) {
    if (!path) {
        printf("usage: rm <path>\n");
        return;
    }

    unlink(path);
}
