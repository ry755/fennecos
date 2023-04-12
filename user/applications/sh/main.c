#include <user/user.h>

#include <stdbool.h>
#include <stdio.h>

void main() {
    printf("hello world!\n");

    char string[512];
    while (true) {
        gets(string);
        printf("string = %s\n", string);
    }
}
