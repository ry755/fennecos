const IDT_ENTRIES = 49;

const IdtEntry = packed struct {
    isr_low: u16,
    kernel_cs: u16,
    reserved: u8,
    attributes: u8,
    isr_high: u16,
};

const IdtPtr = packed struct {
    limit: u16,
    base: *const IdtEntry,
};

var idt: [256]IdtEntry = undefined;
var idt_ptr: IdtPtr = undefined;

extern const isr_stub_table: [IDT_ENTRIES]*void;

pub fn initialize() void {
    idt_ptr.limit = @sizeOf(IdtEntry) * IDT_ENTRIES - 1;
    idt_ptr.base = &idt[0];

    for (0..IDT_ENTRIES) |i|
        set_descriptor(i, isr_stub_table[i], 0x8E);

    asm volatile ("lidt (%[idt_ptr])"
        :
        : [idt_ptr] "r" (&idt_ptr),
    );
    asm volatile ("sti");
}

pub fn set_descriptor(vector: usize, isr: *void, flags: u8) void {
    const descriptor = &idt[vector];
    descriptor.isr_low = @truncate(u16, @ptrToInt(isr) & 0xFFFF);
    descriptor.kernel_cs = 0x08;
    descriptor.attributes = flags;
    descriptor.isr_high = @truncate(u16, @ptrToInt(isr) >> 16);
    descriptor.reserved = 0;
}
