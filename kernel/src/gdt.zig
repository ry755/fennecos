const GDT_ENTRIES = 3;

const GdtEntry = packed struct {
    limit_low: u16,
    base_low: u16,
    base_middle: u8,
    access: u8,
    flags: u8,
    base_high: u8,
};

const GdtPtr = packed struct {
    limit: u16,
    base: *const GdtEntry,
};

var gdt: [GDT_ENTRIES]GdtEntry = undefined;
var gdt_ptr: GdtPtr = undefined;

fn flush_gdt() void {
    asm volatile (
        \\lgdt %[gdt_ptr]
        \\mov $0x10, %ax
        \\mov %ax, %ds
        \\mov %ax, %es
        \\mov %ax, %fs
        \\mov %ax, %gs
        \\mov %ax, %ss
        \\jmp $0x08,$1f
        \\1:
        :
        : [gdt_ptr] "*p" (&gdt_ptr),
        : "ax", "memory"
    );
}

pub fn initialize() void {
    gdt_ptr.limit = (@sizeOf(GdtEntry) * GDT_ENTRIES) - 1;
    gdt_ptr.base = &gdt[0];
    // null segment
    set_entry(0, 0, 0, 0, 0);
    // code segment
    set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0x0C);
    // data segment
    set_entry(2, 0, 0xFFFFFFFF, 0x92, 0x0C);
    flush_gdt();
}

pub fn set_entry(number: u8, base: u32, limit: u32, access: u8, flags: u8) void {
    // base address
    gdt[number].base_low = @truncate(u16, base);
    gdt[number].base_middle = @truncate(u8, (base >> 16) & 0xFF);
    gdt[number].base_high = @truncate(u8, (base >> 24) & 0xFF);
    // limits
    gdt[number].limit_low = @truncate(u16, limit);
    gdt[number].flags = @truncate(u8, (limit >> 16) & 0x0F);
    // flags
    gdt[number].flags |= (flags << 4) & 0xF0;
    // access byte
    gdt[number].access = access;
}
