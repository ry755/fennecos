pub inline fn inb(port: u16) u8 {
    return asm volatile ("inb %[port], %[result]"
        : [result] "={al}" (-> u8),
        : [port] "N{dx}" (port),
    );
}

pub inline fn insl(port: u16, address: *void, count: u32) void {
    asm volatile ("cld; rep insl"
        : [address1] "={edi}" (address),
          [count1] "={ecx}" (count),
        : [port] "{edx}" (port),
          [address2] "0" (address),
          [count2] "1" (count),
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
        : [address1] "={esi}" (address),
          [count1] "={ecx}" (count),
        : [port] "{edx}" (port),
          [address2] "0" (address),
          [count2] "1" (count),
        : "cc"
    );
}
