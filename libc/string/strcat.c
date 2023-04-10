#include <stdint.h>
#include <string.h>

char *strcat(char *dest, const char *src) {
    uint32_t i = 0, j = 0;
    while (dest[i++]);
    i--;
    for (j = 0; src[j]; j++)
        dest[i+j] = src[j];
    dest[i + j] = 0;
    return dest;
}
