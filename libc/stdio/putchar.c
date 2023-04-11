#include <user/user.h>

#include <stdio.h>

int putchar(int ic) {
    char buffer[1];
    char c = (char) ic;
    buffer[0] = c;
    write(1, buffer, 1);
    return ic;
}
