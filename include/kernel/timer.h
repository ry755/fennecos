#pragma once

#include <kernel/trapframe.h>

#include <stdint.h>

void init_timer();
void timer_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error);
uint32_t get_timer_value();
