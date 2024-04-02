const std = @import("std");
const builtin = std.builtin;
const event = @import("event.zig");
const fat = @import("fat.zig");
const gdt = @import("gdt.zig");
const gfx = @import("gfx.zig");
const ide = @import("ide.zig");
const idt = @import("idt.zig");
const isr = @import("isr.zig");
const kbd = @import("kbd.zig");
const mbr = @import("mbr.zig");
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
    mbr.initialize();
    fat.initialize(
        0,
        &ide.read,
        &ide.write,
        mbr.mbr.partitions[0].lba_first_sector,
    );
    winmgr.initialize();

    fat.global_fat.mount("0:", true) catch @panic("failed to mount 0:");
    defer fat.global_fat.unmount("0:");
    var test_file = fat.fatfs.File.openRead("0:/test.txt") catch @panic("failed to open 0:/test.txt");
    defer test_file.close();
    var test_reader = test_file.reader();
    var test_contents: [512]u8 = std.mem.zeroes([512]u8);
    _ = test_reader.read(&test_contents) catch @panic("failed to read 0:/test.txt");
    writer.print("{s}\n", .{test_contents}) catch unreachable;

    writer.print("kernel initialization done, entering event loop\n", .{}) catch unreachable;
    event_loop();
}

pub fn panic(message: []const u8, _: ?*builtin.StackTrace, _: ?usize) noreturn {
    writer.print("\noops!!! a fucky wucky occurred!!!\n{s}\n", .{message}) catch unreachable;
    while (true) {}
}

fn event_loop() noreturn {
    var x: u32 = 8;
    while (true) {
        const e = event.get_next_event();
        switch (e.event_type) {
            .key_down => {
                const scancode: u8 = @truncate(e.parameters[0]);
                const character = kbd.scancode_to_ascii(scancode);
                gfx.move_to(&gfx.Point{ .x = x, .y = 128 });
                gfx.writer.print("{c}", .{character}) catch unreachable;
                x += gfx.default_font.width;
            },
            else => {},
        }
    }
}
