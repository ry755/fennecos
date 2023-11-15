// currently only the PIT is supported

const gfx = @import("gfx.zig");
const io = @import("io.zig");
const isr = @import("isr.zig");
const mouse = @import("mouse.zig");

var ticks: u64 = 0;
var mouse_coords: gfx.Point = undefined;
var mouse_coords_old: gfx.Point = undefined;

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
        mouse_coords = mouse.coordinates;
        if (mouse_coords.x != mouse_coords_old.x or
            mouse_coords.y != mouse_coords_old.y)
        {
            gfx.move_cursor(&mouse_coords);
            mouse_coords_old = mouse_coords;
        }
        gfx.blit_buffered_framebuffer_to_hw();
    }
}

pub fn get_ticks() u64 {
    return ticks;
}
