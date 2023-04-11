#include <kernel/allocator.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

#include <stddef.h>
#include <stdint.h>

extern process_t *current_process;

uint32_t sys_open() {
    uint32_t id = get_unused_file_id();
    if (id == (uint32_t) -1)
        return (uint32_t) -1;
    current_process->files[id] = (file_t *) kallocate(sizeof(file_t), false, NULL);
    if (open(current_process->files[id], (char *) fetch_syscall_u32(0), (uint8_t) fetch_syscall_u32(1)))
        return id;
    else
        return (uint32_t) -1;
}
