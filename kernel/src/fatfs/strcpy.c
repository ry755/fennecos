#include <string.h>

char *strcpy(char *dstptr, const char *str) {
    char *initial = (char *) str;
    while ((*dstptr++ = *str++));
    return initial;
}
