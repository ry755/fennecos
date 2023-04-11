#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// based on itoa() implementation here: https://www.geeksforgeeks.org/implement-itoa/
char *itoa(int value, char *str, int base) {
    int index = 0;
    bool negative = false;

    // handle a zero value
    if (!value) {
        str[index] = '0';
        index++;
        str[index] = '\0';
        return str;
    }

    // was a negative value passed?
    if (value < 0 && base == 10) {
        negative = true;
        value = -value;
    }

    while (value != 0) {
        int mod = value % base;
        str[index++] = (mod > 9)? (mod - 10) + 'A' : mod + '0';
        value = value / base;
    }

    if (negative)
        str[index++] = '-';

    str[index] = '\0';

    // reverse the string
    char *str_start = str;
    char *str_end = str_start + strlen(str) - 1;
    char temp_char;

    while (str_end > str_start) {
        temp_char = *str_start;
        *str_start = *str_end;
        *str_end = temp_char;

        ++str_start;
        --str_end;
    }

    return str;
}