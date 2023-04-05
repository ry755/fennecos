#include <kernel/isr.h>
#include <kernel/pic.h>

#include <stdint.h>
#include <stdio.h>

void interrupt_handler(uint8_t irq, uint32_t error) {
    end_of_interrupt(irq);
}
