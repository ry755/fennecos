const std = @import("std");
const heap = @import("heap.zig");

pub const Event = struct {
    event_type: EventType,
    parameters: [8]u32,
};
pub const EventType = enum(u32) {
    empty,
    key_down,
    key_up,
};

pub const QueueNode = struct {
    data: Event,
    next: ?*QueueNode,
};

var start: ?*QueueNode = null;
var end: ?*QueueNode = null;

pub fn new_event(event: Event) !void {
    const node = try heap.allocator.create(QueueNode);
    node.* = .{ .data = event, .next = null };
    if (end) |e| e.next = node else start = node;
    end = node;
}

pub fn get_next_event() Event {
    const s = start orelse return Event{
        .event_type = EventType.empty,
        .parameters = std.mem.zeroes([8]u32),
    };
    defer heap.allocator.destroy(s);
    if (s.next) |next| {
        start = next;
    } else {
        start = null;
        end = null;
    }
    return s.data;
}
