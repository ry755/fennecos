#pragma once

#include <stdint.h>

#define SYS_exit_process   1
#define SYS_yield_process  2
#define SYS_new_event      3
#define SYS_get_next_event 4

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
