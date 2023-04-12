#include <user/user.h>

#include <fox/string.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 512

typedef struct command_s {
    const char *args[64];
} command_t;

char input_buffer[INPUT_BUFFER_SIZE];

static void get_command(char *buffer) {
    printf("$ ");
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
    command_t command = tokenize(buffer);
    for (uint8_t i = 0; i < 64; i++)
        if (command.args[i] != NULL)
            printf("command.args[%d] = %s\n", i, command.args[i]);
}

void main() {
    while (true) {
        get_command(input_buffer);
        run_command(input_buffer);
    }
}
