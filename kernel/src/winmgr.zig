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

var bg_framebuffer_data = std.mem.zeroes([640 * 480 * 4]u8);
pub var bg_framebuffer = gfx.Framebuffer{
    .next = &test_framebuffer,
    .child = null,
    .data = &bg_framebuffer_data,
    .x = 0,
    .y = 0,
    .width = 640,
    .height = 480,
    .pitch = 640 * 4,
    .bpp = 32,
    .dirty = gfx.Rectangle{ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = false,
};

var test_framebuffer_data = std.mem.zeroes([64 * 64 * 3]u8);
var test_framebuffer = gfx.Framebuffer{
    .next = null,
    .child = null,
    .data = &test_framebuffer_data,
    .x = 8,
    .y = 8,
    .width = 64,
    .height = 64,
    .pitch = 64 * 3,
    .bpp = 24,
    .dirty = gfx.Rectangle{ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = false,
};

pub fn initialize() void {
    gfx.invalidate_whole_framebuffer(&bg_framebuffer);
    gfx.set_framebuffer(&test_framebuffer);
    gfx.invalidate_whole_framebuffer(&test_framebuffer);
    gfx.move_to(&gfx.Point{ .x = 8, .y = 8 });
    gfx.set_color(0xFFFFFF, 0x000000);
    gfx.draw_string("hi!!");
    gfx.main_framebuffer.child = &bg_framebuffer;
}

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
