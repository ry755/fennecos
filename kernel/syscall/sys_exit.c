#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_exit() {
    exit_process();
    return 0;
}
