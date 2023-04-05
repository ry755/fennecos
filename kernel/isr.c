#include <kernel/isr.h>
#include <kernel/pic.h>

#include <stdint.h>
#include <stdio.h>

irq_handler_t irq_handlers[16] = { 0 };

void install_interrupt_handler(uint8_t irq, irq_handler_t handler) {
    irq_handlers[irq] = handler;
}

void uninstall_interrupt_handler(uint8_t irq) {
    irq_handlers[irq] = 0;
}

void interrupt_handler(uint8_t irq, uint32_t error) {
    if (irq < 32) {
        exception_handler(irq, error);
    } else {
        void (*handler)() = irq_handlers[irq - 32];
        if (handler) {
            handler(irq, error);
        } else {
            kprintf("unhandled interrupt! irq: %d, error: %x\n", irq, error);
        }
    }
    end_of_interrupt(irq);
}

void exception_handler(uint8_t irq, uint32_t error) {
    kprintf("fatal exception! irq: %d, error: %x\n", irq, error);
}
