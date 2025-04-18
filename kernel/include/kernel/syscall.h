#pragma once

#include <stdint.h>

#define SYS_exit           1
#define SYS_kill           2
#define SYS_yield          3
#define SYS_sleep          4
#define SYS_new_process    5
#define SYS_get_pid        6
#define SYS_open           7
#define SYS_close          8
#define SYS_read           9
#define SYS_write          10
#define SYS_seek           11
#define SYS_unlink         12
#define SYS_cwd            13
#define SYS_chdir          14
#define SYS_new_event      15
#define SYS_get_next_event 16
#define SYS_get_mouse      17

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
