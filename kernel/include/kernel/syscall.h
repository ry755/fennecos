#pragma once

#include <stdint.h>

#define SYS_draw_string 1

uint32_t syscall(uint32_t syscall, uint32_t esp);
uint32_t fetch_syscall_u32(uint8_t n);
