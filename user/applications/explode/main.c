#include <user/user.h>

#include <stdio.h>
#include <stddef.h>

void main() {
    printf("explodes!!!! teehee\n");
    for (uint8_t i = 0; i < 128; i++) {
        printf("%d\n", i);
        new_process("explode.elf", NULL);
        yield();
    }
}
