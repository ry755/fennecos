#include <kernel/allocator.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_free_memory() {
    free((void *) fetch_syscall_u32(0));
    return 0;
}
