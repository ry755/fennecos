#include <kernel/io.h>
#include <kernel/isr.h>
#include <kernel/ps2.h>
#include <kernel/trapframe.h>

#include <stdint.h>
#include <stdio.h>

void init_ps2_keyboard() {
    install_interrupt_handler(1, ps2_keyboard_interrupt_handler);
}

void ps2_keyboard_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;
    uint8_t status = inb(0x64);
    if (!(status & 1))
        return;
    uint8_t scancode = inb(0x60);
    kprintf("scancode: 0x%x\n", scancode);
}