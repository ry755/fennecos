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
const winmgr = @import("winmgr.zig");
const writer = serial.writer;

export fn kernel_main(multiboot_info: *multiboot.MultibootInfo) void {
    gdt.initialize();
    serial.initialize();
    timer.initialize();
    pic.initialize();
    ide.initialize();
    ps2.initialize();
    mouse.initialize();
    gfx.initialize(
        @truncate(multiboot_info.framebuffer_addr),
        multiboot_info.framebuffer_pitch,
        multiboot_info.framebuffer_bpp,
        0x1E1E2E,
        multiboot_info.framebuffer_red_field_position,
        multiboot_info.framebuffer_green_field_position,
        multiboot_info.framebuffer_blue_field_position,
    );
    idt.initialize();

    writer.print("kernel initialization done\n", .{}) catch unreachable;

    writer.print("{}, {}, {}\n", .{
        multiboot_info.framebuffer_red_field_position / 8,
        multiboot_info.framebuffer_green_field_position / 8,
        multiboot_info.framebuffer_blue_field_position / 8,
    }) catch unreachable;

    winmgr.initialize();
    winmgr.event_loop();
}

pub fn panic(message: []const u8, _: ?*builtin.StackTrace, _: ?usize) noreturn {
    writer.print("\noops!!! a fucky wucky occurred!!!\n{s}\n", .{message}) catch unreachable;
    while (true) {}
}
