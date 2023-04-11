#include <kernel/serial.h>

#include <stdio.h>

int putchar(int ic) {
    char c = (char) ic;
    write_serial(c);
    return ic;
}
