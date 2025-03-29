#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stdint.h>
#include <stdio.h>

extern process_t *current_process;

uint32_t sys_write() {
    uint32_t file = fetch_syscall_u32(0);
    if (file == (uint32_t) -1) {
        printf((char *) fetch_syscall_u32(1), fetch_syscall_u32(2));
        return 0;
    } else {
        return write(
            current_process->files[file],
            (char *) fetch_syscall_u32(1),
            fetch_syscall_u32(2)
        );
    }
}
