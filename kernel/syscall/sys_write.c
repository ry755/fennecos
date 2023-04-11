#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stdint.h>

extern process_t *current_process;

uint32_t sys_write() {
    return write(
        current_process->files[fetch_syscall_u32(0)],
        (char *) fetch_syscall_u32(1),
        fetch_syscall_u32(2)
    );
}
