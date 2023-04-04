#include <kernel/gdt.h>

#include <stdint.h>

struct gdt_entry_s gdt[GDT_ENTRIES];
struct gdt_ptr_s gdt_ptr;

void init_gdt() {
    // GDT pointer and limits
    gdt_ptr.limit = (sizeof(struct gdt_entry_s) * GDT_ENTRIES) - 1;
    gdt_ptr.base = (uint32_t) &gdt;
    // null segment
    set_gdt_entry(0, 0, 0, 0, 0);
    // code segment
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0x0C);
    // data segment
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0x0C);
    flush_gdt();
}

void set_gdt_entry(uint8_t number, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    // base address
    gdt[number].base_low = (base & 0xFFFF);
    gdt[number].base_middle = (base >> 16) & 0xFF;
    gdt[number].base_high = (base >> 24) & 0xFF;
    // limits
    gdt[number].limit_low = (limit & 0xFFFF);
    gdt[number].flags = (limit >> 16) & 0x0F;
    // flags
    gdt[number].flags |= (flags << 4) & 0xF0;
    // access byte
    gdt[number].access = access;
}
