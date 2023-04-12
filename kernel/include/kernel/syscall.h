#pragma once

#include <stdint.h>

#define SYS_exit           1
#define SYS_yield          2
#define SYS_new_process    3
#define SYS_open           4
#define SYS_close          5
#define SYS_read           6
#define SYS_write          7
#define SYS_cwd            8
#define SYS_chdir          9
#define SYS_new_event      10
#define SYS_get_next_event 11

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
