#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stdint.h>

extern process_t *current_process;

uint32_t sys_get_pid() {
    return current_process->pid;
}
