const multiboot = @import("multiboot.zig");

pub const Font = struct { data: [*]const u8, width: u8, height: u8 };
pub const default_font = Font{ .data = @embedFile("font.bin").ptr, .width = 8, .height = 16 };
pub const Coordinates = struct { x: u32, y: u32 };
pub const Framebuffer = struct {
    data: [*]u8,
    width: u32,
    height: u32,
    pitch: u32,
    bpp: u8,
};

pub var full_framebuffer = Framebuffer{
    .data = undefined,
    .width = undefined,
    .height = undefined,
    .pitch = undefined,
    .bpp = undefined,
};

var current_coordinates = Coordinates{ .x = 0, .y = 0 };
var current_font: *Font = undefined;
var current_framebuffer: *Framebuffer = undefined;

pub fn initialize(address: u32, width: u32, height: u32, pitch: u32, bpp: u8, color: u32) void {
    full_framebuffer.data = @ptrFromInt(address);
    full_framebuffer.bpp = bpp;
    full_framebuffer.pitch = pitch;
    full_framebuffer.width = width;
    full_framebuffer.height = height;
    current_framebuffer = &full_framebuffer;
    current_font = @constCast(&default_font);

    // fill the framebuffer with the specified color
    var i: u32 = 0;
    while (i < width * height * bpp / 8) {
        full_framebuffer.data[i] = @truncate(color & 0xFF);
        full_framebuffer.data[i + 1] = @truncate((color >> 8) & 0xFF);
        full_framebuffer.data[i + 2] = @truncate((color >> 16) & 0xFF);
        i += bpp / 8;
    }
}

pub fn move_to(x: u32, y: u32) void {
    current_coordinates.x = x;
    current_coordinates.y = y;
}

pub fn set_font(font: *Font) void {
    current_font = font;
}

pub fn set_framebuffer(framebuffer: *Framebuffer) void {
    current_framebuffer = framebuffer;
}

pub fn draw_font_tile(tile: u8, x: u32, y: u32, foreground_color: u32, background_color: u32, font: *Font) void {
    const font_offset: u32 = @as(u32, font.*.width) * @as(u32, font.*.height) * @as(u32, tile);
    const font_data: [*]const u8 = font.*.data + font_offset;

    for (0..font.height) |y_counter| {
        for (0..font.width) |x_counter| {
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
}

pub fn draw_string(string: []const u8, foreground_color: u32, background_color: u32) void {
    var x_mut = current_coordinates.x;
    for (string) |c| {
        draw_font_tile(c, x_mut, current_coordinates.y, foreground_color, background_color, current_font);
        x_mut += current_font.*.width;
    }
}

pub fn blit_framebuffer_into_framebuffer(source: *Framebuffer, target: *Framebuffer, x: u32, y: u32) void {
    const ymin = y;
    var ymax = ymin + source.*.height;
    const xmin = x;
    var xmax = xmin + source.*.width;
    ymax = if (ymax < target.*.height) ymax else target.*.height;
    xmax = if (xmax < target.*.width) xmax else target.*.width;
    if (ymin >= ymax) return;
    if (xmin >= xmax) return;

    for (ymin..ymax) |y1| {
        for (xmin..xmax) |x1| {
            const index_dst = (x1 + y1 * target.*.width) * (target.*.bpp / 8);
            const index_src = ((x1 - xmin) + (y1 - ymin) * source.*.width) * (source.*.bpp / 8);
            target.*.data[index_dst] = source.*.data[index_src];
            target.*.data[index_dst + 1] = source.*.data[index_src + 1];
            target.*.data[index_dst + 2] = source.*.data[index_src + 2];
        }
    }
}
