const std = @import("std");
const ide = @import("ide.zig");
const writer = @import("serial.zig").writer;

const Partition = extern struct {
    status: u8 align(1),
    chs_first_sector: [3]u8 align(1),
    part_type: u8 align(1),
    chs_last_sector: [3]u8 align(1),
    lba_first_sector: u32 align(1),
    sector_count: u32 align(1),
};

const Mbr = extern struct {
    bootstrap: [446]u8 align(1),
    partitions: [4]Partition align(1),
    signature: [2]u8 align(1),
};

pub var mbr = std.mem.zeroes(Mbr);

// TODO: make this generic, not directly tied to the IDE functions
pub fn initialize() void {
    var sector_buffer = std.mem.zeroes([512]u8);
    ide.read(0, &sector_buffer, 1);
    mbr = std.mem.bytesToValue(Mbr, &sector_buffer);
    if (mbr.signature[0] != 0x55 or mbr.signature[1] != 0xAA)
        writer.print("invalid MBR signature!!!\n", .{}) catch unreachable;
}
