#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stddef.h>
#include <stdint.h>

extern process_t *current_process;

char *sys_cwd() {
    return current_process->current_directory;
}
