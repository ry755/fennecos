#include <user/user.h>

#include <stdio.h>

void main(int argc, char *argv[]) {
    printf("\e[31mHello world!\n");
    printf("\e[36mThis is a demo of FennecOS's console and shell!\n");
    printf("\e[0m");

    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = \"%s\"\n", i, argv[i]);
    }
}
