#include <user/user.h>

#include <stddef.h>
#include <stdio.h>

#include "commands.h"

void help() {
    printf("\e[32mFennecOS shell\e[0m\n\n");
    printf("(in descriptions, $n is argument n)\n");
    printf("\e[32mcommand\e[0m | \e[35mdescription\e[0m\n");
    printf("        |            \n");
    printf("\e[32mcd      \e[0m|\e[35m change current working directory to $1\e[0m\n");
    printf("\e[32mcp      \e[0m|\e[35m copy file $1 to $2\e[0m\n");
    printf("\e[32mhelp    \e[0m|\e[35m print this help text\e[0m\n");
    printf("\e[32mls      \e[0m|\e[35m list all files in the current working directory\e[0m\n");
    printf("\e[32mrm      \e[0m|\e[35m remove file $1\e[0m\n\n");
    printf("type the name of an ELF binary (with or without extension) to launch it\n");
}
