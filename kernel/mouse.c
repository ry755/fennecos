#include <kernel/mouse.h>
#include <kernel/isr.h>
#include <kernel/io.h>
#include <kernel/trapframe.h>
#include <kernel/event.h>

#include <stdint.h>
#include <stdbool.h>

uint32_t state = mouse_state_0;
uint8_t byte_0;
uint8_t byte_1;
uint8_t byte_2;

mouse_buttons_t mouse_buttons = { .left = false, .middle = false, .right = false };
uint16_t mouse_x = 0;
uint16_t mouse_y = 0;

static void update_mouse() {
    int16_t rel_x = byte_1 - ((byte_0 << 4) & 0x100);
    int16_t rel_y = byte_2 - ((byte_0 << 3) & 0x100);
    if (rel_x > 0) mouse_x += rel_x; else if ((int16_t) mouse_x > -rel_x) mouse_x -= (-rel_x);
    if (rel_y > 0) {
        if (mouse_y > rel_y) mouse_y -= rel_y;
    } else mouse_y += (-rel_y);
    if (mouse_x > 640) mouse_x = 640;
    if (mouse_y > 480) mouse_y = 480;

    mouse_buttons_t old_mouse_buttons;
    old_mouse_buttons.left = mouse_buttons.left;
    old_mouse_buttons.middle = mouse_buttons.middle;
    old_mouse_buttons.right = mouse_buttons.right;

    mouse_buttons.left = (byte_0 & 1) != 0;
    mouse_buttons.middle = (byte_0 & 4) != 0;
    mouse_buttons.right = (byte_0 & 2) != 0;

    // TODO: other buttons?
    if (mouse_buttons.left != old_mouse_buttons.left) {
        event_t event;
        event.type = mouse_buttons.left ? MOUSE_DOWN : MOUSE_UP;
        event.arg0 = mouse_x;
        event.arg1 = mouse_y;
        new_event(&event);
    }
}

void init_mouse() {
    outb(0x64, 0x20);
    uint8_t status = inb(0x60);
    status |= 1 << 1;
    status &= ~(1 << 5);
    outb(0x64, 0x60);
    while ((inb(0x64) & 1) != 0);
    outb(0x60, status);
    outb(0x64, 0xD4);
    while ((inb(0x64) & 1) != 0);
    outb(0x60, 0xF4);
    inb(0x60);
    install_interrupt_handler(12, mouse_interrupt_handler);
}

void mouse_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;
    if ((inb(0x64) & 1) == 0) return;
    switch (state) {
        case mouse_state_0: {
            state = mouse_state_1;
            byte_0 = inb(0x60);
            break;
        }
        case mouse_state_1: {
            state = mouse_state_2;
            byte_1 = inb(0x60);
            break;
        }
        case mouse_state_2: {
            state = mouse_state_0;
            byte_2 = inb(0x60);
            update_mouse();
            break;
        }
    }
}
