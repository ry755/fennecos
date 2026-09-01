#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "framebuffer.h"

typedef struct window_s {
    nested_fb_t framebuffer;
    uint16_t width;
    uint16_t height;
    uint16_t x;
    uint16_t y;
} window_t;

void new_window(window_t *window, uint16_t width, uint16_t height, uint16_t x, uint16_t y);
void move_window(window_t *window, uint16_t x, uint16_t y);
