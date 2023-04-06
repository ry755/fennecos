#pragma once

#include <kernel/trapframe.h>

void init_ps2_keyboard();
void ps2_keyboard_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error);
