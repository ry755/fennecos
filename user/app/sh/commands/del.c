#include <user/user.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"

void cmd_del(char *path) {
    if (!path) {
        printf("usage: del <path>\n");
        return;
    }

    unlink(path);
}
