#pragma once

#include <kernel/paging.h>

#include <stdint.h>

typedef struct font_s {
    uint8_t *font;
    uint8_t width;
    uint8_t height;
} font_t;

void init_framebuffer(uint32_t physical_address, uint32_t color, uint8_t *font, uint8_t font_width, uint8_t font_height);
void map_framebuffer(page_directory_t *page_directory);
