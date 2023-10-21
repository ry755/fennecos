const std = @import("std");
const builtin = std.builtin;
const gdt = @import("gdt.zig");
const gfx = @import("gfx.zig");
const ide = @import("ide.zig");
const idt = @import("idt.zig");
const isr = @import("isr.zig");
const multiboot = @import("multiboot.zig");
const pic = @import("pic.zig");
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
    isr.install_handler(0, timer.interrupt_handler);
    idt.initialize();
    gfx.initialize(
        @truncate(multiboot_info.framebuffer_addr),
        multiboot_info.framebuffer_pitch,
        multiboot_info.framebuffer_bpp,
        0x1E1E2E,
    );

    gfx.set_framebuffer(&test_framebuffer);
    gfx.invalidate_whole_framebuffer(&test_framebuffer);
    gfx.move_to(8, 8);
    gfx.draw_string("hi!!", 0xFFFFFF, 0x000000);
    gfx.main_framebuffer.child = &test_framebuffer;

    writer.print("kernel initialization done\n", .{}) catch unreachable;

    while (true) {}
}

pub fn panic(message: []const u8, _: ?*builtin.StackTrace, _: ?usize) noreturn {
    writer.print("\noops!!! a fucky wucky occurred!!!\n{s}\n", .{message}) catch unreachable;
    while (true) {}
}
