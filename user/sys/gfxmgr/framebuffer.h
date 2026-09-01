#include <stdint.h>
#include <stdbool.h>

typedef struct rectangle_s {
    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;
} rectangle_t;

typedef struct nested_fb_s {
    struct nested_fb_s *next;
    struct nested_fb_s *child;
    uint8_t bpp;
    uint16_t width;
    uint16_t height;
    uint32_t pitch;
    uint32_t x;
    uint32_t y;
    uint8_t *data;
    bool has_alpha;
    rectangle_t dirty;
} nested_fb_t;

void invalidate_whole_framebuffer(nested_fb_t *framebuffer);
void invalidate_whole_framebuffer_chain(nested_fb_t *framebuffer);
void invalidate_partial_framebuffer(nested_fb_t *framebuffer, rectangle_t *dirty);
void render(nested_fb_t *source, nested_fb_t *target);
void blit_framebuffer_into_framebuffer(nested_fb_t *source, nested_fb_t *target);
