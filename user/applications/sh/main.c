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
    printf("[%s]$ ", cwd());
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

    // ls
    if (!strcmp(command.args[0], "ls")) {
        ls();
        return;
    }

    // not an internal command? then try running it as a binary
    if (!new_process((char *) command.args[0], (char **) command.args)) {
        strcpy(concat_command, command.args[0]);
        strcat(concat_command, ".elf");
        if (!new_process(concat_command, (char **) command.args))
            printf("failed to run %s\n", command.args[0]);
    }
}

void main() {
    while (true) {
        get_command(input_buffer);
        run_command(input_buffer);
        yield();
    }
}
