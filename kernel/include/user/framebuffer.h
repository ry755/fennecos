#pragma once

#include <stdint.h>

typedef struct font_s {
    uint8_t *font;
    uint8_t width;
    uint8_t height;
} font_t;

void draw_font_tile(char tile, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color, font_t *font);
void draw_string(char *str, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color, font_t *font);
