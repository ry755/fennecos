const std = @import("std");
const builtin = std.builtin;
const gdt = @import("gdt.zig");
const graphics = @import("graphics.zig");
const ide = @import("ide.zig");
const idt = @import("idt.zig");
const isr = @import("isr.zig");
const multiboot = @import("multiboot.zig");
const pic = @import("pic.zig");
const pit = @import("pit.zig");
const serial = @import("serial.zig");
const writer = serial.writer;

var test_framebuffer_data = std.mem.zeroes([64 * 64 * 4]u8);
var test_framebuffer = graphics.Framebuffer{
    .data = &test_framebuffer_data,
    .width = 64,
    .height = 64,
    .pitch = 64 * 4,
    .bpp = 32,
};

export fn kernel_main(multiboot_info: *multiboot.MultibootInfo) void {
    gdt.initialize();
    serial.initialize();
    pit.initialize();
    pic.initialize();
    ide.initialize();
    isr.install_handler(0, nothing); // FIXME: temporary, to avoid "unhandled interrupt" message spam
    idt.initialize();
    graphics.initialize(
        @truncate(multiboot_info.framebuffer_addr),
        multiboot_info.framebuffer_width,
        multiboot_info.framebuffer_height,
        multiboot_info.framebuffer_pitch,
        multiboot_info.framebuffer_bpp,
        0x1E1E2E,
    );

    graphics.set_framebuffer(&test_framebuffer);
    graphics.move_to(8, 8);
    graphics.draw_string("hi!!", 0xFFFFFF, 0x000000);
    graphics.blit_framebuffer_into_framebuffer(&test_framebuffer, &graphics.full_framebuffer, 8, 8);

    writer.print("kernel initialization done\n", .{}) catch unreachable;

    while (true) {}
}

pub fn panic(message: []const u8, _: ?*builtin.StackTrace, _: ?usize) noreturn {
    writer.print("\noops!!! a fucky wucky occurred!!!\n{s}\n", .{message}) catch unreachable;
    while (true) {}
}

pub fn nothing() void {
    // nothing lol
}
