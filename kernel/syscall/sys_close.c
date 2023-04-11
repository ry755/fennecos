#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stddef.h>
#include <stdint.h>

extern process_t *current_process;

uint32_t sys_close() {
    return close(current_process->files[fetch_syscall_u32(0)]);
}
