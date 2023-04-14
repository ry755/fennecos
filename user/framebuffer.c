#include <user/framebuffer.h>

#include <stdint.h>

uint8_t *framebuffer = (void *) 0xF0000000;
uint32_t *framebuffer_pitch = (void *) 0xF1000000;
uint8_t *framebuffer_bpp = (void *) 0xF1000004;

void draw_font_tile(char tile, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color, font_t *font) {
    uint8_t *font_ptr = &(font->font[font->width * font->height * tile]);

    for (uint8_t y_counter = 0; y_counter < font->height; y_counter++) {
        for (uint8_t x_counter = 0; x_counter < font->width; x_counter++) {
            uint8_t font_byte = font_ptr[y_counter * font->width + x_counter];
            uint32_t framebuffer_offset = (y + y_counter) * *framebuffer_pitch + ((x + x_counter) * (*framebuffer_bpp / 8));
            if (font_byte) {
                framebuffer[framebuffer_offset++] = foreground_color & 0xFF;
                framebuffer[framebuffer_offset++] = (foreground_color >> 8) & 0xFF;
                framebuffer[framebuffer_offset++] = (foreground_color >> 16) & 0xFF;
            }
            else {
                framebuffer[framebuffer_offset++] = background_color & 0xFF;
                framebuffer[framebuffer_offset++] = (background_color >> 8) & 0xFF;
                framebuffer[framebuffer_offset++] = (background_color >> 16) & 0xFF;
            }
        }
    }
}

void draw_string(char *str, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color, font_t *font) {
    while (*str) {
        draw_font_tile(*str++, x, y, foreground_color, background_color, font);
        x += font->width;
    }
}
