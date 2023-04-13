#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_sleep() {
    sleep_process(fetch_syscall_u32(0));
    return 0;
}
