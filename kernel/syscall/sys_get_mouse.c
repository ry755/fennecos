#include <kernel/mouse.h>
#include <kernel/syscall.h>

#include <stdint.h>

extern uint16_t mouse_x;
extern uint16_t mouse_y;

uint32_t sys_get_mouse() {
    return (mouse_y << 16) | mouse_x;
}
