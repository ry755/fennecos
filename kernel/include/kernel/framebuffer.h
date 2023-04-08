#pragma once

#include <kernel/paging.h>

#include <stdint.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

void init_framebuffer(uint32_t framebuffer_address, uint32_t color);
void map_framebuffer(page_directory_t *page_directory);
void draw_font_tile(char tile, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color);
void draw_string(char *str, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color);
