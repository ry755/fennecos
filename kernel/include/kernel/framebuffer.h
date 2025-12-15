#pragma once

#include <stdint.h>

typedef struct font_s {
    uint8_t *font;
    uint8_t width;
    uint8_t height;
} font_t;

typedef struct fb_s {
    uint8_t bpp;
    uint16_t width;
    uint16_t height;
    uint32_t pitch;
    uint8_t *framebuffer;
    font_t font;
} fb_t;

uintptr_t init_framebuffer(
    uintptr_t physical_address,
    uint16_t width, uint16_t height,
    uint32_t pitch, uint8_t bpp,
    uint32_t fill_color,
    uint8_t font_width, uint8_t font_height
);
fb_t *get_framebuffer();
