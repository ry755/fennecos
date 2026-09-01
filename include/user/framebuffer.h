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

void set_draw_position(uint16_t x, uint16_t y);
void set_draw_color(uint32_t foreground, uint32_t background);
void set_draw_font(font_t *font);
void draw_font_tile(char tile);
void draw_string(char *str);
font_t *get_font();
font_t *get_global_font();
