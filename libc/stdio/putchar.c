#include <stdio.h>
#include <kernel/serial.h>

int putchar(int ic) {
    char c = (char) ic;
    write_serial(c);
    return ic;
}
