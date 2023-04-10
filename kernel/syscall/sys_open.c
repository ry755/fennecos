#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stdint.h>

uint32_t sys_open() {
    return open(
        (file_t *) fetch_syscall_u32(0),
        (char *) fetch_syscall_u32(1),
        (uint8_t) fetch_syscall_u32(2)
    );
}
