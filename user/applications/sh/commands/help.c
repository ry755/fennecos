#include <user/user.h>

#include <stddef.h>
#include <stdio.h>

#include "commands.h"

void cmd_help() {
    // TODO: make this an array of help text for each command, maybe an array of argument names too?
    printf("\e[32mFennecOS shell\e[0m\n\n");
    printf("\e[32mcommand\e[0m           | \e[35mdescription\e[0m\n");
    printf("\e[32mcd <path>         \e[0m|\e[35m change current working directory to <path>\e[0m\n");
    printf("\e[32mcopy <src> <dest> \e[0m|\e[35m copy file <src> to <dest>\e[0m\n");
    printf("\e[32mhelp              \e[0m|\e[35m print this help text\e[0m\n");
    printf("\e[32mdir               \e[0m|\e[35m list all files in the current working directory\e[0m\n");
    printf("\e[32mdel <path>        \e[0m|\e[35m remove file <path>\e[0m\n\n");
    printf("type the name of a .app binary (with or without extension) to launch it\n");
}
