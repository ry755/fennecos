#include <stdint.h>
#include <string.h>

#include <user/user.h>
#include <fox/debug.h>

void debug(char *str, uint32_t value) {
    // calling write with -1 (0xFFFFFFFF) as the fd is treated as a debug print
    // `write()`'s third argument is used as a value to be used in the output
    // `str` is a standard printf-format string
    write((uint32_t) -1, str, value);
}
