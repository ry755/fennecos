#pragma once

#include <kernel/trapframe.h>

#include <stdint.h>
#include <stdbool.h>

enum mouse_state_e {
    mouse_state_0,
    mouse_state_1,
    mouse_state_2
};

typedef struct mouse_buttons_s {
    bool left;
    bool middle;
    bool right;
} mouse_buttons_t;

void init_mouse();
void mouse_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error);
