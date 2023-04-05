#include <kernel/isr.h>
#include <kernel/timer.h>

#include <stdint.h>

uint32_t timer;

void init_timer() {
    timer = 0;
    install_interrupt_handler(0, timer_interrupt_handler);
}

void timer_interrupt_handler() {
    timer++;
}

uint32_t get_timer_value() {
    return timer;
}
