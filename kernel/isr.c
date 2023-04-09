#include <kernel/isr.h>
#include <kernel/pic.h>
#include <kernel/syscall.h>
#include <kernel/trapframe.h>

#include <stdint.h>
#include <stdio.h>

irq_handler_t irq_handlers[16] = { 0 };

void install_interrupt_handler(uint8_t irq, irq_handler_t handler) {
    irq_handlers[irq] = handler;
}

void uninstall_interrupt_handler(uint8_t irq) {
    irq_handlers[irq] = 0;
}

void interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    if (irq < 32) {
        exception_handler(irq, error);
    } else if (irq == 48) {
        // syscall !!
        trap_frame->eax = syscall(trap_frame->eax, trap_frame->esp);
    } else {
        void (*handler)(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) = irq_handlers[irq - 32];
        if (handler) {
            handler(irq, trap_frame, error);
        } else {
            kprintf("unhandled interrupt! irq: %d, error: 0x%x\n", irq, error);
        }
    }
    end_of_interrupt(irq);
}

void exception_handler(uint8_t irq, uint32_t error) {
    kprintf("fatal exception! irq: %d, error: 0x%x\n", irq, error);
    switch (irq) {
        case 14:
            // page fault
            uint32_t virtual_address;
            asm volatile ("movl %%cr2, %0" : "=a"(virtual_address));
            kprintf("page fault occurred while accessing virtual address 0x%x", virtual_address);
            break;

        default:
            break;
    }
}
