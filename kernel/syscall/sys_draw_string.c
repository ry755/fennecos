#include <kernel/framebuffer.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_draw_string() {
    draw_string(
        (char *) fetch_syscall_u32(0),
        fetch_syscall_u32(1),
        fetch_syscall_u32(2),
        fetch_syscall_u32(3),
        fetch_syscall_u32(4)
    );
    return 0;
}
