#include <user/user.h>
#include <user/framebuffer.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

fb_t *framebuffer = NULL;
uint16_t fb_draw_x = 0;
uint16_t fb_draw_y = 0;
uint32_t fb_draw_bg = 0x1E1E2E;
uint32_t fb_draw_fg = 0xCDD6F4;
font_t *fb_draw_font_override = NULL;

static void init_framebuffer() {
    framebuffer = (fb_t *) get_fb();
    fb_draw_x = 0;
    fb_draw_y = 0;
    fb_draw_bg = 0x1E1E2E;
    fb_draw_fg = 0xCDD6F4;
    fb_draw_font_override = NULL;
}

void set_draw_position(uint16_t x, uint16_t y) {
    fb_draw_x = x;
    fb_draw_y = y;
}

void set_draw_color(uint32_t foreground, uint32_t background) {
    fb_draw_fg = foreground;
    fb_draw_bg = background;
}

void set_draw_font(font_t *font) {
    if (framebuffer == NULL) init_framebuffer();
    fb_draw_font_override = font;
}

void draw_font_tile(char tile) {
    if (framebuffer == NULL) init_framebuffer();

    font_t *font = (fb_draw_font_override == NULL) ? &framebuffer->font : fb_draw_font_override;
    uint8_t *font_ptr = &(font->font[font->width * font->height * tile]);

    for (uint8_t y_counter = 0; y_counter < font->height; y_counter++) {
        for (uint8_t x_counter = 0; x_counter < font->width; x_counter++) {
            uint8_t font_byte = font_ptr[y_counter * font->width + x_counter];
            uint32_t framebuffer_offset = (fb_draw_y + y_counter) * framebuffer->pitch + ((fb_draw_x + x_counter) * (framebuffer->bpp / 8));
            if (font_byte) {
                framebuffer->framebuffer[framebuffer_offset++] = fb_draw_fg & 0xFF;
                framebuffer->framebuffer[framebuffer_offset++] = (fb_draw_fg >> 8) & 0xFF;
                framebuffer->framebuffer[framebuffer_offset++] = (fb_draw_fg >> 16) & 0xFF;
            } else {
                framebuffer->framebuffer[framebuffer_offset++] = fb_draw_bg & 0xFF;
                framebuffer->framebuffer[framebuffer_offset++] = (fb_draw_bg >> 8) & 0xFF;
                framebuffer->framebuffer[framebuffer_offset++] = (fb_draw_bg >> 16) & 0xFF;
            }
        }
    }
}

void draw_string(char *str) {
    if (framebuffer == NULL) init_framebuffer();

    font_t *font = (fb_draw_font_override == NULL) ? &framebuffer->font : fb_draw_font_override;
    uint16_t orig_x = fb_draw_x;
    while (*str) {
        switch (*str) {
            case '\n':
                fb_draw_y += font->height;
                // fall through
            case '\r':
                fb_draw_x = orig_x;
                break;
            default:
                draw_font_tile(*str);
                fb_draw_x += font->width;
                break;
        }
        str++;
    }
}

font_t *get_font() {
    if (framebuffer == NULL) init_framebuffer();
    return (fb_draw_font_override == NULL) ? &framebuffer->font : fb_draw_font_override;
}

font_t *get_global_font() {
    if (framebuffer == NULL) init_framebuffer();
    return &framebuffer->font;
}
