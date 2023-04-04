#pragma once

#include <stdint.h>

#define GDT_ENTRIES 3

struct gdt_entry_s {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_s {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern void flush_gdt();
void set_gdt_entry(uint8_t number, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void init_gdt();
