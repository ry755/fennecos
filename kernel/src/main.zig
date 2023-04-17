const std = @import("std");
const serial = @import("serial.zig");
const gdt = @import("gdt.zig");
const multiboot = @import("multiboot.zig");

export fn kernel_main(multiboot_info: multiboot.MultibootInfo) void {
    _ = multiboot_info;
    gdt.initialize();
    serial.initialize();
    serial.write_string("hello world!\n");
}
