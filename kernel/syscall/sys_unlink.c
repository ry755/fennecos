#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stddef.h>
#include <stdint.h>

uint32_t sys_unlink() {
    return unlink((char *) fetch_syscall_u32(0));
}
