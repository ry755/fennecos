#include <user/user.h>
#include <fox/debug.h>
#include <fox/string.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 512
#define RC_PATH "/sys/init.rc"

typedef struct command_s {
    const char *args[64];
} command_t;

char line[INPUT_BUFFER_SIZE];

static command_t tokenize(char *buffer) {
    command_t command;
    memset(&command, 0, sizeof(command_t));

    string_t source = string_from(buffer);
    string_t **tokens = string_tokenize(source);

    for (int i = 0; tokens[i] != NULL; i++) {
        command.args[i] = tokens[i]->data;
    }

    return command;
}

void main() {
    uint32_t rc_file = open(RC_PATH, MODE_READ);
    if (!rc_file) {
        debug("init: failed to open %s\n", (uint32_t) RC_PATH);
        exit();
    }

    uint32_t read_ret;
    do {
        uint16_t offset = 0;
        do {
            read_ret = read(rc_file, &line[offset], 1);
        } while ((line[offset++] != '\n') && (read_ret != 0));
        if (offset > 0) line[offset - 1] = 0;
        if (offset < 3) {
            // probably not a valid path (i.e. end of rc file), ignore it
            continue;
        }

        command_t command = tokenize(line);

        if (!new_process((char *) command.args[0], (char **) command.args)) {
            debug("init: failed to run %s\n", (uint32_t) command.args[0]);
            exit();
        }

        yield();
    } while (read_ret != 0);

    close(rc_file);
    debug("init: done\n", 0);
}
