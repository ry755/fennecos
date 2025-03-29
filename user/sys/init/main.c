#include <user/user.h>

#include <stddef.h>
#include <stdio.h>

void main() {
    uint32_t rc_file = open("/sys/init.rc", MODE_READ);
    if (!rc_file) exit();

    char line[256];
    uint16_t offset = 0;
    uint32_t read_ret;
    do {
        read_ret = read(rc_file, &line[offset], 1);
    } while ((line[offset] != '\n') && (read_ret != 0));
    // TODO: finish this

    if (!new_process("sh.app", NULL)) {
        exit();
    }
}
