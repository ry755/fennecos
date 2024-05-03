#pragma once

#include <stdint.h>

#define SYS_exit           1
#define SYS_yield          2
#define SYS_sleep          3
#define SYS_new_process    4
#define SYS_open           5
#define SYS_close          6
#define SYS_read           7
#define SYS_write          8
#define SYS_seek           9
#define SYS_unlink         10
#define SYS_cwd            11
#define SYS_chdir          12
#define SYS_new_event      13
#define SYS_get_next_event 14
#define SYS_get_mouse      15

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
