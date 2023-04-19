const std = @import("std");
const serial = @import("serial.zig");
const gdt = @import("gdt.zig");
const idt = @import("idt.zig");
const isr = @import("isr.zig");
const multiboot = @import("multiboot.zig");
const pic = @import("pic.zig");
const pit = @import("pit.zig");

export fn kernel_main(multiboot_info: multiboot.MultibootInfo) void {
    _ = multiboot_info;
    gdt.initialize();
    serial.initialize();
    pit.initialize();
    pic.initialize();
    isr.uninstall_handler(0);
    idt.initialize();
    while (true) {}
}
