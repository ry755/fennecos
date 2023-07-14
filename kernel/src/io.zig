pub inline fn inb(port: u16) u8 {
    return asm volatile ("inb %[port], %[result]"
        : [result] "={al}" (-> u8),
        : [port] "N{dx}" (port),
    );
}

pub inline fn insl(port: u16, address: *void, count: u32) void {
    asm volatile ("cld; rep insl"
        : [address] "=D" (address),
          [count] "=c" (count),
        : [port] "d" (port),
          [address] "0" (address),
          [count] "1" (count),
        : "memory", "cc"
    );
}

pub inline fn outb(port: u16, value: u8) void {
    asm volatile ("outb %[value], %[port]"
        :
        : [value] "{al}" (value),
          [port] "N{dx}" (port),
    );
}

pub inline fn outsl(port: u16, address: *void, count: u32) void {
    asm volatile ("cld; rep outsl"
        : [address] "=S" (address),
          [count] "=c" (count),
        : [port] "d" (port),
          [address] "0" (address),
          [count] "1" (count),
        : "cc"
    );
}
