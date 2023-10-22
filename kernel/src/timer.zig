// currently only the PIT is supported

const gfx = @import("gfx.zig");
const io = @import("io.zig");
const isr = @import("isr.zig");

var ticks: u64 = 0;

pub fn initialize() void {
    // mode 2
    io.outb(0x43, 0x36);

    // 100 Hz
    io.outb(0x40, 11931 & 0x00FF);
    io.outb(0x40, (11931 & 0xFF00) >> 8);

    isr.install_handler(0, interrupt_handler);
}

pub fn interrupt_handler() void {
    ticks = @addWithOverflow(ticks, 1)[0];
    if (ticks % 2 == 0) {
        gfx.blit_buffered_framebuffer_to_hw();
    }
}

pub fn get_ticks() u64 {
    return ticks;
}
