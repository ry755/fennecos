#include <kernel/allocator.h>
#include <kernel/framebuffer.h>

#include <stdint.h>

fb_t global_framebuffer;

uintptr_t init_framebuffer(
        uintptr_t physical_address,
        uint16_t width, uint16_t height,
        uint32_t pitch, uint8_t bpp,
        uint32_t fill_color,
        uint8_t font_width, uint8_t font_height
    ) {
    global_framebuffer.framebuffer = (uint8_t *) physical_address;
    global_framebuffer.width = width;
    global_framebuffer.height = height;
    global_framebuffer.pitch = pitch;
    if (bpp > 100) bpp /= 10; // hack for some (one?) device which reports 24 bpp as 240 bpp
    global_framebuffer.bpp = bpp;

    // fill the framebuffer with the specified color
    for (uint32_t i = 0; i < 640 * 480 * bpp / 8; i += bpp / 8) {
        global_framebuffer.framebuffer[i] = fill_color & 0xFF;
        global_framebuffer.framebuffer[i + 1] = (fill_color >> 8) & 0xFF;
        global_framebuffer.framebuffer[i + 2] = (fill_color >> 16) & 0xFF;
    }

    global_framebuffer.font.font = allocate(font_width * font_height * 256, false);
    global_framebuffer.font.width = font_width;
    global_framebuffer.font.height = font_height;

    return (uintptr_t) global_framebuffer.font.font;
}

fb_t *get_framebuffer() {
    return &global_framebuffer;
}
