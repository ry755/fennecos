const std = @import("std");

const Writer = std.io.Writer(@TypeOf(.{}), error{}, draw_string_writer);
pub const writer = Writer{ .context = .{} };

pub const Font = struct { data: []const u8, width: u8, height: u8 };
pub const default_font = Font{ .data = @embedFile("font.bin")[0..], .width = 8, .height = 16 };
pub const Point = struct { x: u32, y: u32 };
pub const Rectangle = struct { x1: u32, y1: u32, x2: u32, y2: u32 };
pub const Framebuffer = struct {
    next: ?*Framebuffer,
    child: ?*Framebuffer,
    data: [*]u8,
    x: u32,
    y: u32,
    width: u32,
    height: u32,
    pitch: u32,
    bpp: u8,
    dirty: Rectangle,
};

var hw_framebuffer = Framebuffer{
    .next = null,
    .child = &main_framebuffer,
    .data = undefined,
    .x = 0,
    .y = 0,
    .width = 640,
    .height = 480,
    .pitch = undefined,
    .bpp = undefined,
    .dirty = Rectangle{ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
};

var main_framebuffer_data = std.mem.zeroes([640 * 480 * 4]u8);
pub var main_framebuffer = Framebuffer{
    .next = null,
    .child = null,
    .data = &main_framebuffer_data,
    .x = 0,
    .y = 0,
    .width = 640,
    .height = 480,
    .pitch = 640 * 4,
    .bpp = 32,
    .dirty = Rectangle{ .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
};

var current_coordinates = Point{ .x = 0, .y = 0 };
var current_foreground_color: u32 = 0xFFFFFF;
var current_background_color: u32 = 0x000000;
var current_font: *Font = undefined;
var current_framebuffer: *Framebuffer = undefined;

pub fn initialize(address: u32, pitch: u32, bpp: u8, color: u32) void {
    hw_framebuffer.data = @ptrFromInt(address);
    hw_framebuffer.bpp = bpp;
    hw_framebuffer.pitch = pitch;
    current_framebuffer = &main_framebuffer;
    current_font = @constCast(&default_font);

    // fill the framebuffer with the specified color
    var i: u32 = 0;
    while (i < @as(u32, 640) * @as(u32, 480) * bpp / 8) {
        main_framebuffer.data[i] = @truncate(color & 0xFF);
        main_framebuffer.data[i + 1] = @truncate((color >> 8) & 0xFF);
        main_framebuffer.data[i + 2] = @truncate((color >> 16) & 0xFF);
        i += bpp / 8;
    }

    invalidate_whole_framebuffer(&main_framebuffer);
}

pub fn move_to(x: u32, y: u32) void {
    current_coordinates.x = x;
    current_coordinates.y = y;
}

pub fn set_color(foreground: u32, background: u32) void {
    current_foreground_color = foreground;
    current_background_color = background;
}

pub fn set_font(font: *Font) void {
    current_font = font;
}

pub fn set_framebuffer(framebuffer: *Framebuffer) void {
    current_framebuffer = framebuffer;
}

pub fn invalidate_whole_framebuffer(framebuffer: *Framebuffer) void {
    framebuffer.*.dirty.x1 = 0;
    framebuffer.*.dirty.y1 = 0;
    framebuffer.*.dirty.x2 = framebuffer.*.width;
    framebuffer.*.dirty.y2 = framebuffer.*.height;
}

pub fn draw_font_tile(tile: u8, x: u32, y: u32, foreground_color: u32, background_color: u32, font: *Font) void {
    const font_offset: u32 = @as(u32, font.*.width) * @as(u32, font.*.height) * @as(u32, tile);
    const font_data = font.*.data.ptr + font_offset;

    for (0..font.*.height) |y_counter| {
        for (0..font.*.width) |x_counter| {
            const font_byte = font_data[y_counter * font.*.width + x_counter];
            const framebuffer_offset = (y + y_counter) * current_framebuffer.*.pitch + ((x + x_counter) * (current_framebuffer.*.bpp / 8));
            if (font_byte != 0) {
                current_framebuffer.*.data[framebuffer_offset] = @truncate(foreground_color & 0xFF);
                current_framebuffer.*.data[framebuffer_offset + 1] = @truncate((foreground_color >> 8) & 0xFF);
                current_framebuffer.*.data[framebuffer_offset + 2] = @truncate((foreground_color >> 16) & 0xFF);
            } else {
                current_framebuffer.*.data[framebuffer_offset] = @truncate(background_color & 0xFF);
                current_framebuffer.*.data[framebuffer_offset + 1] = @truncate((background_color >> 8) & 0xFF);
                current_framebuffer.*.data[framebuffer_offset + 2] = @truncate((background_color >> 16) & 0xFF);
            }
        }
    }

    // if our dirty rectangle is not set, then set it directly to our coords and size
    if (current_framebuffer.dirty.x1 == 0 and
        current_framebuffer.dirty.y1 == 0 and
        current_framebuffer.dirty.x2 == 0 and
        current_framebuffer.dirty.y2 == 0)
    {
        current_framebuffer.dirty.x1 = x;
        current_framebuffer.dirty.y1 = y;
        current_framebuffer.dirty.x2 = x + font.*.width;
        current_framebuffer.dirty.y2 = y + font.*.height;
    } else {
        // we already have a dirty rectangle, integrate our newly drawn font tile into it
        current_framebuffer.dirty.x1 = if (current_framebuffer.dirty.x1 < x) current_framebuffer.dirty.x1 else x;
        current_framebuffer.dirty.y1 = if (current_framebuffer.dirty.y1 < y) current_framebuffer.dirty.y1 else y;
        current_framebuffer.dirty.x2 = if (current_framebuffer.dirty.x2 > x + font.*.width) current_framebuffer.dirty.x2 else x + font.*.width;
        current_framebuffer.dirty.y2 = if (current_framebuffer.dirty.y2 > y + font.*.height) current_framebuffer.dirty.y2 else y + font.*.height;
    }
}

pub fn draw_string(string: []const u8) void {
    var x_mut = current_coordinates.x;
    for (string) |c| {
        draw_font_tile(c, x_mut, current_coordinates.y, current_foreground_color, current_background_color, current_font);
        x_mut += current_font.*.width;
    }
}

pub fn render(source: ?*Framebuffer, target: ?*Framebuffer) void {
    if (source == null or target == null) return;

    var current = source;
    var old = current;

    while (true) {
        old = current;

        render(current.?.child, current);
        blit_framebuffer_into_framebuffer(current.?, target.?);

        current = current.?.next;
        if (old.?.next == null) break;
    }
}

pub fn blit_buffered_framebuffer_to_hw() void {
    render(&main_framebuffer, &hw_framebuffer);
    hw_framebuffer.dirty.x1 = 0;
    hw_framebuffer.dirty.y1 = 0;
    hw_framebuffer.dirty.x2 = 0;
    hw_framebuffer.dirty.y2 = 0;
}

fn blit_framebuffer_into_framebuffer(source: *Framebuffer, target: *Framebuffer) void {
    const x = source.*.x;
    const y = source.*.y;

    const ymin = y + source.*.dirty.y1;
    var ymax = y + source.*.dirty.y2;
    const xmin = x + source.*.dirty.x1;
    var xmax = x + source.*.dirty.x2;
    ymax = if (ymax < target.*.height) ymax else target.*.height;
    xmax = if (xmax < target.*.width) xmax else target.*.width;
    if (ymin >= ymax) return;
    if (xmin >= xmax) return;

    for (ymin..ymax) |y1| {
        for (xmin..xmax) |x1| {
            const index_dst = (x1 + y1 * target.*.width) * (target.*.bpp / 8);
            const index_src = ((x1 - x) + (y1 - y) * source.*.width) * (source.*.bpp / 8);
            target.*.data[index_dst] = source.*.data[index_src];
            target.*.data[index_dst + 1] = source.*.data[index_src + 1];
            target.*.data[index_dst + 2] = source.*.data[index_src + 2];
        }
    }

    // if the target's dirty rectangle is not set, then set it directly to our coords and size
    if (target.*.dirty.x1 == 0 and
        target.*.dirty.y1 == 0 and
        target.*.dirty.x2 == 0 and
        target.*.dirty.y2 == 0)
    {
        target.*.dirty.x1 = xmin;
        target.*.dirty.y1 = ymin;
        target.*.dirty.x2 = xmax;
        target.*.dirty.y2 = ymax;
    } else {
        // we already have a dirty rectangle, integrate our blit into it
        target.*.dirty.x1 = if (target.*.dirty.x1 < xmin) target.*.dirty.x1 else xmin;
        target.*.dirty.y1 = if (target.*.dirty.y1 < ymin) target.*.dirty.y1 else ymin;
        target.*.dirty.x2 = if (target.*.dirty.x2 > xmax) target.*.dirty.x2 else xmax;
        target.*.dirty.y2 = if (target.*.dirty.y2 > ymax) target.*.dirty.y2 else ymax;
    }

    // clear the source dirty rectangle
    source.*.dirty.x1 = 0;
    source.*.dirty.y1 = 0;
    source.*.dirty.x2 = 0;
    source.*.dirty.y2 = 0;
}

fn draw_string_writer(_: @TypeOf(.{}), string: []const u8) error{}!usize {
    draw_string(string);
    return string.len;
}
