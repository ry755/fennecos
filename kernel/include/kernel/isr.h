#pragma once

#include <stdint.h>

typedef void (*irq_handler_t)();

void install_interrupt_handler(uint8_t irq, irq_handler_t handler);
void uninstall_interrupt_handler(uint8_t irq);
void interrupt_handler(uint8_t irq, uint32_t error);
void exception_handler(uint8_t irq, uint32_t error);
