#include <kernel/allocator.h>
#include <kernel/framebuffer.h>
#include <kernel/paging.h>

#include <stdint.h>

uint8_t *framebuffer = (void *) 0xF0000000;
uint32_t *framebuffer_pitch = (void *) 0xF012C000;
uint8_t *framebuffer_bpp = (void *) 0xF012C004;
uint32_t physical_framebuffer_address;
font_t *global_font = (void *) 0xFF000000;
uint8_t global_font_bitmap_buffer[16 * 16 * 256] __attribute__((aligned (4096)));
font_t global_font_buffer __attribute__((aligned (4096)));

extern page_directory_t *kernel_page_directory;

void init_framebuffer(uint32_t physical_address, uint32_t pitch, uint8_t bpp, uint32_t color, uint8_t *font, uint8_t font_width, uint8_t font_height) {
    // map the framebuffer starting at 0xF0000000
    physical_framebuffer_address = physical_address;
    map_framebuffer(kernel_page_directory);
    *framebuffer_pitch = pitch;
    if (bpp > 100) bpp /= 10; // hack for some (one?) device which reports 24 bpp as 240 bpp
    *framebuffer_bpp = bpp;

    // fill the framebuffer with the specified color
    for (uint32_t i = 0; i < 640 * 480 * bpp / 8; i += bpp / 8) {
        framebuffer[i] = color & 0xFF;
        framebuffer[i + 1] = (color >> 8) & 0xFF;
        framebuffer[i + 2] = (color >> 16) & 0xFF;
    }

    // copy the font
    global_font->font = (uint8_t *) &global_font_bitmap_buffer;
    for (uint32_t i = 0; i < font_width * font_height * 256; i++)
        global_font->font[i] = font[i];
    global_font->width = font_width;
    global_font->height = font_height;
}

void map_framebuffer(page_directory_t *page_directory) {
    uint32_t virtual_address = (uint32_t) framebuffer;
    uint32_t physical_address = physical_framebuffer_address;
    for (uint16_t i = 0; i < 301; i++) {
        map_physical_to_virtual(page_directory, physical_address, virtual_address, true, true);
        physical_address += 0x1000;
        virtual_address += 0x1000;
    }
    map_physical_to_virtual(page_directory, (uint32_t) &global_font_buffer, (uint32_t) global_font, true, true);
    for (uint32_t i = 0; i < 16; i++)
        map_physical_to_virtual(page_directory, (uint32_t) &global_font_bitmap_buffer + (i * 0x1000), 0xFF100000 + (i * 0x1000), true, true);
}
