#include <kernel/framebuffer.h>

#include <stdint.h>

uint8_t font[FONT_WIDTH * FONT_HEIGHT * 256];
uint32_t *framebuffer;

void init_framebuffer(uint32_t framebuffer_address, uint32_t color) {
    framebuffer = (void *)framebuffer_address;
    for (uint32_t i = 0; i < 640*480; i++) {
        framebuffer[i] = color;
    }
}

void draw_font_tile(char tile, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color) {
    uint8_t *font_ptr = &font[FONT_WIDTH * FONT_HEIGHT * tile];
    uint32_t framebuffer_offset = y * 640 + x;

    for (uint8_t y_counter = 0; y_counter < FONT_HEIGHT; y_counter++) {
        for (uint8_t x_counter = 0; x_counter < FONT_WIDTH; x_counter++) {
            uint8_t font_byte = font_ptr[y_counter * FONT_WIDTH + x_counter];
            if (font_byte)
                framebuffer[framebuffer_offset++] = foreground_color;
            else
                framebuffer[framebuffer_offset++] = background_color;
        }
        framebuffer_offset = framebuffer_offset - FONT_WIDTH + 640;
    }
}

void draw_string(char *str, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color) {
    while (*str) {
        draw_font_tile(*str++, x, y, foreground_color, background_color);
        x += FONT_WIDTH;
    }
}
