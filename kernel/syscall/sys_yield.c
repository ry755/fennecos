#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_yield() {
    yield_process();
    return 0;
}
