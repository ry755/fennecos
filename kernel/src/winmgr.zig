const std = @import("std");
const event = @import("event.zig");
const gfx = @import("gfx.zig");
const heap = @import("heap.zig");

const Window = struct {
    x: u32,
    y: u32,
    width: u32,
    height: u32,
    event_start: ?*event.QueueNode,
    event_end: ?*event.QueueNode,
};

pub fn new_event(window: *Window, window_event: event.Event) !void {
    const node = try heap.allocator.create(event.QueueNode);
    node.* = .{ .data = window_event, .next = null };
    if (window.event_end) |e| e.next = node else window.event_start = node;
    window.event_end = node;
}

pub fn get_next_event(window: *Window) event.Event {
    const s = window.event_start orelse return event.Event{
        .event_type = event.EventType.empty,
        .parameters = std.mem.zeroes([8]u32),
    };
    defer heap.allocator.destroy(s);
    if (s.next) |next| {
        window.event_start = next;
    } else {
        window.event_start = null;
        window.event_end = null;
    }
    return s.data;
}
