#include <stdarg.h>
#include <stdio.h>

int kprintf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);

    int written_prefix = printf("[kernel] ");
    int written = vprintf(format, parameters);

    va_end(parameters);
    return written_prefix + written;
}
