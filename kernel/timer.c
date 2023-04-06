#include <kernel/isr.h>
#include <kernel/timer.h>
#include <kernel/trapframe.h>

#include <stdint.h>

uint32_t timer;

void init_timer() {
    timer = 0;
    install_interrupt_handler(0, timer_interrupt_handler);
}

void timer_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;
    timer++;
}

uint32_t get_timer_value() {
    return timer;
}
