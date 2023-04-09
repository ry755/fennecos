#include <kernel/syscall.h>
#include <kernel/trapframe.h>

#include <stdint.h>

uint32_t current_syscall_esp;

extern uint32_t sys_exit_process();
extern uint32_t sys_yield_process();
extern uint32_t sys_new_event();
extern uint32_t sys_get_next_event();
extern uint32_t sys_draw_string();

static uint32_t (*syscalls[])(void) = {
    [SYS_exit_process]   sys_exit_process,
    [SYS_yield_process]  sys_yield_process,
    [SYS_new_event]      sys_new_event,
    [SYS_get_next_event] sys_get_next_event,
    [SYS_draw_string]    sys_draw_string,
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
