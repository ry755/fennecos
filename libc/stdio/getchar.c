#include <user/user.h>

#include <stdio.h>

int getchar() {
    char buffer[1] = { 0 };
    while (!buffer[0]) {
        yield();
        read(0, buffer, 1);
    }
    return (int) buffer[0];
}
