#include <user/user.h>
#include <fox/debug.h>
#include <fox/string.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MAX_PROCESSES 32

#define INPUT_BUFFER_SIZE 512
#define RC_PATH "/sys/init.rc"

typedef struct command_s {
    const char *args[64];
} command_t;

char line[INPUT_BUFFER_SIZE];

uint32_t pids[MAX_PROCESSES] = { 0 };

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

static void init_debug(char *str, char *arg) {
    debug("init: ", 0);
    debug(str, (uint32_t) arg);
    printf("init: ");
    printf(str, arg);
}

void main() {
    if (get_pid() != 0) {
        init_debug("not pid 0\n", NULL);
        exit();
    }

    uint32_t rc_file = open(RC_PATH, MODE_READ);
    if (!rc_file) {
        init_debug("failed to open %s\n", RC_PATH);
        exit();
    }

    uint32_t process_count = 0;
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

        uint32_t pid = new_process((char *) command.args[0], (char **) command.args);
        if (!pid) {
            init_debug("failed to run %s\n", (char *) command.args[0]);
            continue;
        }
        pids[process_count++] = pid;
    } while (read_ret != 0);

    close(rc_file);
    init_debug("rc done\n", NULL);

    while (true) {
        uint32_t process_count = 0;
        // watch the pids that we started and see if they exit
        // if so, remove them from our pid list
        // pids of 0 will never occur since our pid should always be 0
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (pids[i]) {
                if (!kill(pids[i], SIGNAL_CHECK)) {
                    // remove the pid from the list
                    pids[i] = 0;
                } else {
                    process_count++;
                }
            }
            yield();
        }
        if (process_count == 0) {
            init_debug("exiting\n", NULL);
            break;
        }
    }
}
