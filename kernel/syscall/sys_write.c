#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stdint.h>

uint32_t sys_write() {
    return write(
        (file_t *) fetch_syscall_u32(0),
        (char *) fetch_syscall_u32(1),
        fetch_syscall_u32(2)
    );
}
