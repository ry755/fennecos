#include <kernel/process.h>
#include <kernel/syscall.h>

#include <fatfs/ff.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern process_t *current_process;

uint32_t sys_chdir() {
    char *dir_path = (char *) fetch_syscall_u32(0);
    return chdir(dir_path);
}
