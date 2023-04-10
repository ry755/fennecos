#include <user/framebuffer.h>

#include <stdint.h>

uint32_t *framebuffer = (void *) 0xF0000000;

void draw_font_tile(char tile, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color, font_t *font) {
    uint8_t *font_ptr = &(font->font[font->width * font->height * tile]);
    uint32_t framebuffer_offset = y * 640 + x;


    for (uint8_t y_counter = 0; y_counter < font->height; y_counter++) {
        for (uint8_t x_counter = 0; x_counter < font->width; x_counter++) {
            uint8_t font_byte = font_ptr[y_counter * font->width + x_counter];
            if (font_byte)
                framebuffer[framebuffer_offset++] = foreground_color;
            else
                framebuffer[framebuffer_offset++] = background_color;
        }
        framebuffer_offset = framebuffer_offset - font->width + 640;
    }
}

void draw_string(char *str, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color, font_t *font) {
    while (*str) {
        draw_font_tile(*str++, x, y, foreground_color, background_color, font);
        x += font->width;
    }
}
