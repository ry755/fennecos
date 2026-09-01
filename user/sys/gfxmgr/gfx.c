#include <user/user.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "framebuffer.h"
#include "gfx.h"
#include "window.h"

uint16_t current_x = 0;
uint16_t current_y = 0;
uint32_t current_bg = 0x1E1E2E;
uint32_t current_fg = 0xCDD6F4;
window_t *current_port = NULL;
font_t *current_font = NULL;

void gfx_move_to(uint16_t x, uint16_t y) {
    current_x = x;
    current_y = y;
}

void gfx_set_port(window_t *window) {
    current_port = window;
}

void gfx_set_draw_color(uint32_t foreground, uint32_t background) {
    current_fg = foreground;
    current_bg = background;
}

void gfx_draw_font_tile(char tile) {
    if (current_font == NULL) current_font = get_global_font();

    font_t *font = current_font;
    uint8_t *font_ptr = &(font->font[font->width * font->height * tile]);

    for (uint8_t y_counter = 0; y_counter < font->height; y_counter++) {
        for (uint8_t x_counter = 0; x_counter < font->width; x_counter++) {
            uint8_t font_byte = font_ptr[y_counter * font->width + x_counter];
            uint32_t framebuffer_offset = (current_y + y_counter) * current_port->framebuffer.pitch + ((current_x + x_counter) * (current_port->framebuffer.bpp / 8));
            if (font_byte) {
                current_port->framebuffer.data[framebuffer_offset++] = current_fg & 0xFF;
                current_port->framebuffer.data[framebuffer_offset++] = (current_fg >> 8) & 0xFF;
                current_port->framebuffer.data[framebuffer_offset++] = (current_fg >> 16) & 0xFF;
            } else {
                current_port->framebuffer.data[framebuffer_offset++] = current_bg & 0xFF;
                current_port->framebuffer.data[framebuffer_offset++] = (current_bg >> 8) & 0xFF;
                current_port->framebuffer.data[framebuffer_offset++] = (current_bg >> 16) & 0xFF;
            }
        }
    }

    rectangle_t dirty_rect = {.x1 = current_x, .y1 = current_y, .x2 = current_x + font->width, .y2 = current_y + font->height};
    invalidate_partial_framebuffer(&current_port->framebuffer, &dirty_rect);
}

void gfx_draw_string(char *str) {
    font_t *font = get_font();
    uint16_t orig_x = current_x;
    while (*str) {
        switch (*str) {
            case '\n':
                current_y += font->height;
                // fall through
            case '\r':
                current_x = orig_x;
                break;
            default:
                set_draw_position(current_x, current_y);
                gfx_draw_font_tile(*str);
                current_x += font->width;
                if (current_x >= current_port->width) {
                    current_x = 0;
                    current_y += font->height;
                }
                break;
        }
        str++;
    }
}
