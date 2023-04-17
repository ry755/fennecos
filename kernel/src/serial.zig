const io = @import("io.zig");

const COM1_PORT = 0x03F8;

pub fn initialize() void {
    io.outb(COM1_PORT + 1, 0x00); // disable all interrupts
    io.outb(COM1_PORT + 3, 0x80); // enable DLAB (set baud rate divisor)
    io.outb(COM1_PORT + 0, 0x03); // set divisor to 3 (lo byte) 38400 baud
    io.outb(COM1_PORT + 1, 0x00); //                  (hi byte)
    io.outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    io.outb(COM1_PORT + 2, 0xC7); // enable FIFO, clear them, with 14-byte threshold
    io.outb(COM1_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    io.outb(COM1_PORT + 4, 0x1E); // set in loopback mode, test the serial chip
    io.outb(COM1_PORT + 0, 0xAE); // test serial chip (send byte 0xAE and check if serial returns same byte)

    if (io.inb(COM1_PORT) != 0xAE)
        return;

    io.outb(COM1_PORT + 4, 0x0F);
}

fn is_transmit_empty() u8 {
    return io.inb(COM1_PORT + 5) & 0x20;
}

pub fn write(character: u8) void {
    while (is_transmit_empty() == 0) {}
    io.outb(COM1_PORT, character);
}

pub fn write_string(string: []const u8) void {
    for (string) |char|
        write(char);
}
