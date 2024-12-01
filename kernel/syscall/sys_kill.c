#include <kernel/process.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_kill() {
    return kill_process(
        fetch_syscall_u32(0),
        fetch_syscall_u32(1)
    );
}
