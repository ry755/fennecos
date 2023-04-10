#pragma once

#include <stdint.h>

#define SYS_exit           1
#define SYS_yield          2
#define SYS_open           3
#define SYS_read           4
#define SYS_write          5
#define SYS_new_event      6
#define SYS_get_next_event 7

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
