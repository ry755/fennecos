const io = @import("io.zig");

pub fn initialize() void {
    // setup ICW 1
    io.outb(0x20, 0x11);
    io.outb(0xA0, 0x11);

    // setup ICW 2
    io.outb(0x21, 0x20); // IRQ 0 mapped to interrupt 0x20
    io.outb(0xA1, 0x28); // IRQ 8 mapped to interrupt 0x28

    // setup ICW 3
    io.outb(0x21, 0x04); // use IRQ 2 to communicate with the secondary PIC
    io.outb(0xA1, 0x02);

    // setup ICW 4
    io.outb(0x21, 0x01); // enable x86 mode
    io.outb(0xA1, 0x01);

    io.outb(0x21, 0x00);
    io.outb(0xA1, 0x00);
}

pub fn end_of_interrupt(irq: u8) void {
    if (irq >= 8)
        io.outb(0xA0, 0x20);
    io.outb(0x20, 0x20);
}
