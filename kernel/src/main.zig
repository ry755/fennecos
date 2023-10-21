const std = @import("std");
const builtin = std.builtin;
const event = @import("event.zig");
const gdt = @import("gdt.zig");
const gfx = @import("gfx.zig");
const ide = @import("ide.zig");
const idt = @import("idt.zig");
const isr = @import("isr.zig");
const kbd = @import("kbd.zig");
const mouse = @import("mouse.zig");
const multiboot = @import("multiboot.zig");
const pic = @import("pic.zig");
const ps2 = @import("ps2.zig");
const serial = @import("serial.zig");
const timer = @import("timer.zig");
const writer = serial.writer;

var test_framebuffer_data = std.mem.zeroes([64 * 64 * 4]u8);
var test_framebuffer = gfx.Framebuffer{
    .next = null,
    .child = null,
    .data = &test_framebuffer_data,
    .x = 8,
    .y = 8,
    .width = 64,
    .height = 64,
    .pitch = 64 * 4,
    .bpp = 32,
    .dirty = gfx.Rectangle{ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
};

export fn kernel_main(multiboot_info: *multiboot.MultibootInfo) void {
    gdt.initialize();
    serial.initialize();
    timer.initialize();
    pic.initialize();
    ide.initialize();
    idt.initialize();
    ps2.initialize();
    mouse.initialize();
    gfx.initialize(
        @truncate(multiboot_info.framebuffer_addr),
        multiboot_info.framebuffer_pitch,
        multiboot_info.framebuffer_bpp,
        0x1E1E2E,
    );

    gfx.set_framebuffer(&test_framebuffer);
    gfx.invalidate_whole_framebuffer(&test_framebuffer);
    gfx.move_to(8, 8);
    gfx.set_color(0xFFFFFF, 0x000000);
    gfx.draw_string("hi!!");
    gfx.main_framebuffer.child = &test_framebuffer;

    writer.print("kernel initialization done\n", .{}) catch unreachable;

    gfx.set_framebuffer(&gfx.main_framebuffer);
    var x: u32 = 8;
    var mouse_coords = mouse.coordinates;
    var mouse_coords_old = mouse_coords;
    while (true) {
        const e = event.get_next_event();
        switch (e.event_type) {
            .key_down => {
                const scancode: u8 = @truncate(e.parameters[0]);
                const character = kbd.scancode_to_ascii(scancode);
                gfx.move_to(x, 128);
                gfx.writer.print("{c}", .{character}) catch unreachable;
                x += gfx.default_font.width;
            },
            else => {},
        }

        mouse_coords = mouse.coordinates;
        if (mouse_coords.x != mouse_coords_old.x or
            mouse_coords.y != mouse_coords_old.y)
        {
            mouse_coords_old = mouse_coords;
            writer.print("x: {}, y: {}\n", .{ mouse_coords.x, mouse_coords.y }) catch unreachable;
        }
    }
}

pub fn panic(message: []const u8, _: ?*builtin.StackTrace, _: ?usize) noreturn {
    writer.print("\noops!!! a fucky wucky occurred!!!\n{s}\n", .{message}) catch unreachable;
    while (true) {}
}
