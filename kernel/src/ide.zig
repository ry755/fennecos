const io = @import("io.zig");
const isr = @import("isr.zig");

const SECTOR_SIZE = 512;

pub fn initialize() void {
    isr.install_handler(14, interrupt_handler);
}

pub fn read(sector: u32, buffer: [*]u8, count: u32) void {
    while (count) {
        read_sector(buffer, sector);
        sector += 1;
        buffer += SECTOR_SIZE;
        count -= 1;
    }
}

pub fn write(sector: u32, buffer: [*]u8, count: u32) void {
    while (count) {
        write_sector(buffer, sector);
        sector += 1;
        buffer += SECTOR_SIZE;
        count -= 1;
    }
}

fn interrupt_handler() void {
    // nothing
}

fn wait() void {
    // wait for the disk to become ready
    while ((io.inb(0x1F7) & 0xC0) != 0x40) {}
}

fn read_sector(destination: *void, sector: u32) void {
    wait();
    io.outb(0x1F2, 1);
    io.outb(0x1F3, @truncate(sector));
    io.outb(0x1F4, @truncate(sector >> 8));
    io.outb(0x1F5, @truncate(sector >> 16));
    io.outb(0x1F6, @truncate((sector >> 24) | 0xE0));
    io.outb(0x1F7, 0x20);

    wait();
    io.insl(0x1F0, destination, SECTOR_SIZE / 4);
}

fn write_sector(source: *void, sector: u32) void {
    wait();
    io.outb(0x1F2, 1);
    io.outb(0x1F3, @truncate(sector));
    io.outb(0x1F4, @truncate(sector >> 8));
    io.outb(0x1F5, @truncate(sector >> 16));
    io.outb(0x1F6, @truncate((sector >> 24) | 0xE0));
    io.outb(0x1F7, 0x30);

    wait();
    io.outsl(0x1F0, source, SECTOR_SIZE / 4);
}
