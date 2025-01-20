#include <user/user.h>

#include <fox/string.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands/commands.h"

#define INPUT_BUFFER_SIZE 512

typedef struct command_s {
    const char *args[64];
} command_t;

char input_buffer[INPUT_BUFFER_SIZE];

static void get_command(char *buffer) {
    printf("[%s]> ", cwd());
    memset(buffer, 0, INPUT_BUFFER_SIZE);
    gets(buffer);
}

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

static void run_command(char *buffer) {
    char concat_command[512];
    if (!buffer[0]) return;
    command_t command = tokenize(buffer);
    if (!command.args[0]) return;

    // cd
    if (!strcmp(command.args[0], "cd")) {
        if (command.args[1]) {
            if (!chdir((char *) command.args[1]))
                printf("could not change working directory to %s\n", command.args[1]);
        } else {
            printf("must provide a path\n");
        }
        return;
    }

    // copy
    if (!strcmp(command.args[0], "copy")) {
        cmd_copy((char *) command.args[1], (char *) command.args[2]);
        return;
    }

    // help
    if (!strcmp(command.args[0], "help")) {
        cmd_help();
        return;
    }

    // dir
    if (!strcmp(command.args[0], "dir")) {
        cmd_dir();
        return;
    }

    // del
    if (!strcmp(command.args[0], "del")) {
        cmd_del((char *) command.args[1]);
        return;
    }

    // not an internal command? then try running it as a binary
    uint32_t pid = new_process((char *) command.args[0], (char **) command.args);
    if (!pid) {
        strcpy(concat_command, command.args[0]);
        strcat(concat_command, ".app");
        pid = new_process(concat_command, (char **) command.args);
        if (!pid) {
            printf("failed to run %s\n", command.args[0]);
            return;
        }
    }
    if (pid) {
        while (kill(pid, SIGNAL_CHECK)) {
            yield();
        }
    }
}

void main() {
    while (true) {
        get_command(input_buffer);
        run_command(input_buffer);
        yield();
    }
}
