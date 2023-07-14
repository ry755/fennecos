const std = @import("std");
const gdt = @import("gdt.zig");
const ide = @import("ide.zig");
const idt = @import("idt.zig");
const isr = @import("isr.zig");
const multiboot = @import("multiboot.zig");
const pic = @import("pic.zig");
const pit = @import("pit.zig");
const serial = @import("serial.zig");

export fn kernel_main(multiboot_info: multiboot.MultibootInfo) void {
    _ = multiboot_info;
    gdt.initialize();
    serial.initialize();
    pit.initialize();
    pic.initialize();
    ide.initialize();
    isr.install_handler(0, nothing); // FIXME: temporary, to avoid "unhandled interrupt" message spam
    idt.initialize();

    serial.writer.print("kernel initialization done\n", .{}) catch unreachable;

    while (true) {}
}

pub fn nothing() void {
    // nothing lol
}
