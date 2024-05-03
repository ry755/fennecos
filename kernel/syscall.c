#include <kernel/syscall.h>
#include <kernel/trapframe.h>

#include <stdint.h>

uint32_t current_syscall_esp;

extern uint32_t sys_exit();
extern uint32_t sys_yield();
extern uint32_t sys_sleep();
extern uint32_t sys_new_process();
extern uint32_t sys_open();
extern uint32_t sys_close();
extern uint32_t sys_read();
extern uint32_t sys_write();
extern uint32_t sys_seek();
extern uint32_t sys_unlink();
extern uint32_t sys_cwd();
extern uint32_t sys_chdir();
extern uint32_t sys_new_event();
extern uint32_t sys_get_next_event();
extern uint32_t sys_get_mouse();

static uint32_t (*syscalls[])(void) = {
    [SYS_exit]           sys_exit,
    [SYS_yield]          sys_yield,
    [SYS_sleep]          sys_sleep,
    [SYS_new_process]    sys_new_process,
    [SYS_open]           sys_open,
    [SYS_close]          sys_close,
    [SYS_read]           sys_read,
    [SYS_write]          sys_write,
    [SYS_seek]           sys_seek,
    [SYS_unlink]         sys_unlink,
    [SYS_cwd]            sys_cwd,
    [SYS_chdir]          sys_chdir,
    [SYS_new_event]      sys_new_event,
    [SYS_get_next_event] sys_get_next_event,
    [SYS_get_mouse]      sys_get_mouse,
};

static uint32_t fetch_u32(uint32_t address) {
    return *(uint32_t *) address;
}

uint32_t syscall(uint32_t syscall, uint32_t esp) {
    asm volatile ("sti");
    current_syscall_esp = esp;
    return syscalls[syscall]();
}

uint32_t fetch_syscall_u32(uint8_t n) {
    // fetch nth syscall argument
    return fetch_u32(current_syscall_esp + 32 + (4 * n));
}
