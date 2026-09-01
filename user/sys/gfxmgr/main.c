#include <user/user.h>
#include <fox/debug.h>
#include <fox/string.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "cursor.h"

#include "framebuffer.h"

nested_fb_t cursor_fb = {
    .next = NULL,
    .child = NULL,
    .data = (uint8_t *) &cursor_data[0],
    .x = 16,
    .y = 16,
    .width = 8,
    .height = 12,
    .pitch = 8 * 4,
    .bpp = 32,
    .dirty = { .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = true,
};

uint8_t main_fb_data[640*480*4];
nested_fb_t main_fb = {
    .next = &cursor_fb,
    .child = NULL,
    .data = main_fb_data,
    .x = 0,
    .y = 0,
    .width = 640,
    .height = 480,
    .pitch = 640 * 4,
    .bpp = 32,
    .dirty = { .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = false,
};

fb_t *hw_fb;
nested_fb_t hw_nested_fb = {
    .next = NULL,
    .child = &main_fb,
    .data = NULL,
    .x = 0,
    .y = 0,
    .width = 640,
    .height = 480,
    .pitch = 640 * 4,
    .bpp = 32,
    .dirty = { .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = false,
};

void main() {
    hw_fb = (fb_t *) get_fb();
    hw_nested_fb.data = hw_fb->framebuffer;
    hw_nested_fb.pitch = hw_fb->pitch;
    hw_nested_fb.bpp = hw_fb->bpp;

    memset(main_fb_data, 0x22, 640*480*4); // fill with a grey-ish color
    invalidate_whole_framebuffer_chain(&main_fb);

    uint32_t mouse = 0;
    while (true) {
        rectangle_t mouse_dirty_rect = {
            .x1 = cursor_fb.x,
            .x2 = cursor_fb.x + cursor_fb.width,
            .y1 = cursor_fb.y,
            .y2 = cursor_fb.y + cursor_fb.height
        };
        invalidate_partial_framebuffer(&main_fb, &mouse_dirty_rect);
        invalidate_whole_framebuffer(&cursor_fb);

        mouse = get_mouse();
        cursor_fb.x = mouse & 0x0000FFFF;
        cursor_fb.y = mouse >> 16;
        render(&main_fb, &hw_nested_fb);
    }
}
