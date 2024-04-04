#include <kernel/allocator.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stddef.h>
#include <stdint.h>

extern process_t *current_process;

uint32_t sys_close() {
    size_t file = fetch_syscall_u32(0);
    bool closed = close(current_process->files[file]);
    if (closed) {
        free(current_process->files[file]);
        current_process->files[file] = 0;
    }
    return closed;
}
