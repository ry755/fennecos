const std = @import("std");
const event = @import("event.zig");
const gfx = @import("gfx.zig");
const kbd = @import("kbd.zig");
const mouse = @import("mouse.zig");

var bg_framebuffer_data = @embedFile("bg.raw").*;
var bg_framebuffer = gfx.Framebuffer{
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
    .next = &cursor_framebuffer,
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

var cursor_framebuffer_data = @embedFile("cursor.raw").*;
var cursor_framebuffer = gfx.Framebuffer{
    .next = null,
    .child = null,
    .data = &cursor_framebuffer_data,
    .x = 0,
    .y = 0,
    .width = 8,
    .height = 12,
    .pitch = 8 * 4,
    .bpp = 32,
    .dirty = gfx.Rectangle{ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = true,
};

pub fn initialize() void {
    gfx.invalidate_whole_framebuffer(&bg_framebuffer);
    gfx.set_framebuffer(&test_framebuffer);
    gfx.invalidate_whole_framebuffer(&test_framebuffer);
    gfx.move_to(8, 8);
    gfx.set_color(0xFFFFFF, 0x000000);
    gfx.draw_string("hi!!");
    gfx.main_framebuffer.child = &bg_framebuffer;
}

pub fn event_loop() noreturn {
    gfx.set_framebuffer(&gfx.main_framebuffer);
    var x: u32 = 8;
    var mouse_coords = mouse.coordinates;
    var mouse_coords_old = mouse_coords;
    while (true) {
        const e = event.get_next_event();
        switch (e.event_type) {
            .key_down => {
                const scancode: u8 = @truncate(e.parameters[0]);
                const character = kbd.scancode_to_ascii(scancode);
                gfx.move_to(x, 128);
                gfx.writer.print("{c}", .{character}) catch unreachable;
                x += gfx.default_font.width;
            },
            else => {},
        }

        mouse_coords = mouse.coordinates;
        if (mouse_coords.x != mouse_coords_old.x or
            mouse_coords.y != mouse_coords_old.y)
        {
            cursor_framebuffer.x = mouse_coords.x;
            cursor_framebuffer.y = mouse_coords.y;
            gfx.invalidate_partial_framebuffer(&bg_framebuffer, &gfx.Rectangle{
                .x1 = mouse_coords_old.x,
                .y1 = mouse_coords_old.y,
                .x2 = mouse_coords_old.x + 16,
                .y2 = mouse_coords_old.y + 16,
            });
            gfx.invalidate_whole_framebuffer_chain(bg_framebuffer.next); // FIXME: don't invalidate the *entire* `next`
            mouse_coords_old = mouse_coords;
        }
    }
}
