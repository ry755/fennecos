#include <kernel/syscall.h>
#include <kernel/trapframe.h>

#include <stdint.h>

uint32_t current_syscall_esp;

extern uint32_t sys_draw_string();

static uint32_t (*syscalls[])(void) = {
    [SYS_draw_string] sys_draw_string,
};

static uint32_t fetch_u32(uint32_t address) {
    return *(uint32_t *) address;
}

uint32_t syscall(uint32_t syscall, uint32_t esp) {
    current_syscall_esp = esp;
    return syscalls[syscall]();
}

uint32_t fetch_syscall_u32(uint8_t n) {
    // fetch nth syscall argument
    return fetch_u32(current_syscall_esp + 32 + (4 * n));
}
