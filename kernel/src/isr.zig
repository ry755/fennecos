const std = @import("std");
const pic = @import("pic.zig");
const stackframe = @import("stackframe.zig");
const writer = @import("serial.zig").writer;

const IrqHandler = ?*const fn () void;

var irq_handlers = std.mem.zeroes([16]IrqHandler);

pub fn install_handler(irq: u8, handler: IrqHandler) void {
    irq_handlers[irq] = handler;
}

pub fn uninstall_handler(irq: u8) void {
    irq_handlers[irq] = null;
}

export fn interrupt_handler(irq: u8, stack_frame: *stackframe.StackFrame, err: u32) void {
    _ = stack_frame;
    if (irq < 32) {
        exception_handler(irq, err);
    } else if (irq == 48) {
        // syscall
    } else {
        const handler = irq_handlers[irq - 32];
        if (handler) |handler_fn| {
            handler_fn();
        } else {
            writer.print("unhandled interrupt! irq: {}, error: {}\n", .{ irq, err }) catch unreachable;
        }
    }
    pic.end_of_interrupt(irq);
}

fn exception_handler(irq: u8, err: u32) void {
    writer.print("fatal exception! irq: {}, error: {}\n", .{ irq, err }) catch unreachable;
}
