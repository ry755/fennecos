#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void) {
    // TODO: Add proper kernel panic.
    kprintf("kernel panic: abort()\n");
    while (1) { }
    __builtin_unreachable();
}
