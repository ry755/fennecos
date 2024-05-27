const gfx = @import("gfx.zig");
const io = @import("io.zig");
const isr = @import("isr.zig");

const MouseState = enum {
    mouse_state_0,
    mouse_state_1,
    mouse_state_2,
};

const Buttons = struct {
    left: bool,
    middle: bool,
    right: bool,
};

var state = MouseState.mouse_state_0;
var byte_0: u8 = undefined;
var byte_1: u8 = undefined;
var byte_2: u8 = undefined;

pub var coordinates = gfx.Point{ .x = 0, .y = 0 };
pub var buttons = Buttons{ .left = false, .middle = false, .right = false };

pub fn initialize() void {
    io.outb(0x64, 0xAD);
    mouse_wait(false);
    io.outb(0x64, 0xA8);
    mouse_wait(false);

    io.outb(0x64, 0xD4);
    mouse_wait(false);
    io.outb(0x60, 0xF6);
    mouse_wait(true);
    _ = io.inb(0x60);

    mouse_wait(false);
    io.outb(0x64, 0x20);
    mouse_wait(true);
    var status = io.inb(0x60);
    status |= 1 << 1;
    status &= ~@as(u8, 1 << 5);
    mouse_wait(false);
    io.outb(0x64, 0x60);
    mouse_wait(false);
    io.outb(0x60, status);
    mouse_wait(false);
    io.outb(0x64, 0xD4);
    mouse_wait(false);
    io.outb(0x60, 0xF4);
    mouse_wait(true);
    _ = io.inb(0x60);
    mouse_wait(false);
    io.outb(0x64, 0xAE);
    _ = io.inb(0x60);
    _ = io.inb(0x60);
    _ = io.inb(0x60);
    _ = io.inb(0x60);

    isr.install_handler(12, interrupt_handler);
}

pub fn interrupt_handler() void {
    if (io.inb(0x64) & 1 == 0) return;
    switch (state) {
        .mouse_state_0 => {
            state = MouseState.mouse_state_1;
            byte_0 = io.inb(0x60);
        },
        .mouse_state_1 => {
            state = MouseState.mouse_state_2;
            byte_1 = io.inb(0x60);
        },
        .mouse_state_2 => {
            state = MouseState.mouse_state_0;
            byte_2 = io.inb(0x60);
            update();
        },
    }
}

fn update() void {
    const rel_x: i8 = @truncate(byte_1 - (@as(i16, ((byte_0 << 4))) & 0x100));
    const rel_y: i8 = @truncate(byte_2 - (@as(i16, ((byte_0 << 3))) & 0x100));
    if (rel_x > 0) coordinates.x += @intCast(rel_x) else if (coordinates.x > -rel_x) coordinates.x -= @intCast(-rel_x);
    if (rel_y > 0) {
        if (coordinates.y > rel_y) coordinates.y -= @intCast(rel_y);
    } else coordinates.y += @intCast(-rel_y);
    if (coordinates.x > 640) coordinates.x = 640;
    if (coordinates.y > 480) coordinates.y = 480;
    buttons.left = byte_0 & 1 != 0;
    buttons.middle = byte_0 & 4 != 0;
    buttons.right = byte_0 & 2 != 0;
}

fn mouse_wait(read: bool) void {
    var timeout: u32 = 100000;
    if (read) {
        while (timeout != 0) {
            if ((io.inb(0x64) & 1) != 0)
                return;
            timeout -= 1;
        }
    } else {
        while (timeout != 0) {
            if ((io.inb(0x64) & 2) != 0)
                return;
            timeout -= 1;
        }
    }
}
