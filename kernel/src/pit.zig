const io = @import("io.zig");

pub fn initialize() void {
    // mode 2
    io.outb(0x43, 0x36);

    // 100 Hz
    io.outb(0x40, 11931 & 0x00FF);
    io.outb(0x40, (11931 & 0xFF00) >> 8);
}
