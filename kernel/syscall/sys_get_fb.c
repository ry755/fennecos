#include <kernel/framebuffer.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_get_fb() {
    return (uint32_t) get_framebuffer();
}
