#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stddef.h>
#include <stdint.h>

uint32_t sys_new_process() {
    return new_process(
        (char *) fetch_syscall_u32(0),
        (char **) fetch_syscall_u32(1),
        NULL,
        NULL
    );
}
