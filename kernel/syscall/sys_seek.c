#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stdint.h>

extern process_t *current_process;

uint32_t sys_seek() {
    return seek(
        current_process->files[fetch_syscall_u32(0)],
        fetch_syscall_u32(1)
    );
}
