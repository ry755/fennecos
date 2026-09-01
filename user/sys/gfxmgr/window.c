#include <user/user.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "framebuffer.h"
#include "window.h"

extern nested_fb_t main_fb;

uint8_t bg_fb_data[640*480*4];
nested_fb_t bg_fb = {
    .next = NULL,
    .child = NULL,
    .data = bg_fb_data,
    .x = 0,
    .y = 0,
    .width = 640,
    .height = 480,
    .pitch = 640 * 4,
    .bpp = 32,
    .dirty = { .x1 = 0, .y1 = 0, .x2 = 0, .y2 = 0 },
    .has_alpha = false,
};

void new_window(window_t *window, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    // find the end of the window chain
    nested_fb_t *window_chain = &bg_fb;
    nested_fb_t *window_chain_end;
    while (window_chain != NULL) {
        window_chain_end = window_chain;
        window_chain = window_chain->next;
    }

    window_chain_end->next = &window->framebuffer;

    window->width = width;
    window->height = height;
    window->x = x;
    window->y = y;

    window->framebuffer.next = NULL;
    window->framebuffer.child = NULL;
    window->framebuffer.bpp = 32;
    window->framebuffer.width = width;
    window->framebuffer.height = height;
    window->framebuffer.pitch = width * 4;
    window->framebuffer.x = x;
    window->framebuffer.y = y;
    window->framebuffer.has_alpha = false;
    window->framebuffer.data = alloc_memory(width * height * 4);

    invalidate_whole_framebuffer(&window->framebuffer);
}

void move_window(window_t *window, uint16_t x, uint16_t y) {
    window->x = x;
    window->y = y;
    window->framebuffer.x = x;
    window->framebuffer.y = y;
    invalidate_whole_framebuffer_chain(&bg_fb);
}
