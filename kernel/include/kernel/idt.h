#pragma once

#include <stdint.h>

#define IDT_ENTRIES 45

typedef struct {
    uint16_t isr_low;   // lower 16 bits of the ISR's address
    uint16_t kernel_cs; // GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t reserved;   // zero
    uint8_t attributes; // type and attributes; see the IDT page
    uint16_t isr_high;  // higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

void init_idt();
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
