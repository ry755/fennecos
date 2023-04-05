#include <kernel/idt.h>

#include <stdbool.h>
#include <stdint.h>

extern void *isr_stub_table[];

__attribute__((aligned(0x10)))
static idt_entry_t idt[256];
static idtr_t idtr;

void init_idt() {
    idtr.base = (uintptr_t) &idt[0];
    idtr.limit = (uint16_t) sizeof(idt_entry_t) * IDT_ENTRIES - 1;

    for (uint8_t vector = 0; vector < IDT_ENTRIES; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }

    __asm__ volatile ("lidt %0" :: "m" (idtr));
    __asm__ volatile ("sti");
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];
    descriptor->isr_low = (uint32_t) isr & 0xFFFF;
    descriptor->kernel_cs = 0x08;
    descriptor->attributes = flags;
    descriptor->isr_high = (uint32_t) isr >> 16;
    descriptor->reserved = 0;
}
