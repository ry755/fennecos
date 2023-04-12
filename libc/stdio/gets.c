#include <stdio.h>

char *gets(char *str) {
    char c;
    do {
        c = getchar();
        *str = c;
        str++;
    } while (c != '\n');
    str--;
    *str = 0;
    return str;
}
