const std = @import("std");
const event = @import("event.zig");
const io = @import("io.zig");
const isr = @import("isr.zig");
const kbd = @import("kbd.zig");
const writer = @import("serial.zig").writer;

pub fn initialize() void {
    isr.install_handler(1, interrupt_handler);
}

pub fn interrupt_handler() void {
    if ((io.inb(0x64) & 1) == 0) return;

    const scancode = io.inb(0x60);
    kbd.keyboard_event(scancode);
    var event_params = std.mem.zeroes([8]u32);
    event_params[0] = scancode;
    const key_event = event.Event{
        .event_type = if (scancode & 0x80 != 0) event.EventType.key_up else event.EventType.key_down,
        .parameters = event_params,
    };
    event.new_event(key_event) catch writer.print("failed to create new keyboard event!\n", .{}) catch unreachable;
}
