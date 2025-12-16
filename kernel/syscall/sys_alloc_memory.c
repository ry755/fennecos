#include <kernel/allocator.h>
#include <kernel/syscall.h>

#include <stdbool.h>
#include <stdint.h>

uint32_t sys_alloc_memory() {
    return (uintptr_t) allocate(fetch_syscall_u32(0), false);
}
