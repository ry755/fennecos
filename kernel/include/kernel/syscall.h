#pragma once

#include <stdint.h>

#define SYS_yield_process 1
#define SYS_draw_string   2

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
