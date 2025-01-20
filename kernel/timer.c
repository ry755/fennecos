#include <kernel/floppy.h>
#include <kernel/isr.h>
#include <kernel/timer.h>
#include <kernel/trapframe.h>

#include <stdint.h>

volatile uint32_t timer;

extern volatile uint32_t floppy_motor_ticks;
extern volatile uint32_t floppy_motor_state;

void init_timer() {
    timer = 0;
    install_interrupt_handler(0, timer_interrupt_handler);
}

void timer_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;

    if ((floppy_motor_state == floppy_motor_wait) && (floppy_motor_ticks != 0)) {
        floppy_motor_ticks--;
        if (floppy_motor_ticks == 0) {
            floppy_motor_kill(FLOPPY_BASE);
        }
    }

    timer++;
}

uint32_t get_timer_value() {
    return timer;
}
