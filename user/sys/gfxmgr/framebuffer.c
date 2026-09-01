#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "framebuffer.h"

void invalidate_whole_framebuffer(nested_fb_t *framebuffer) {
    framebuffer->dirty.x1 = 0;
    framebuffer->dirty.y1 = 0;
    framebuffer->dirty.x2 = framebuffer->width;
    framebuffer->dirty.y2 = framebuffer->height;
}

void invalidate_whole_framebuffer_chain(nested_fb_t *framebuffer) {
    if (framebuffer == NULL) return;

    nested_fb_t *current = framebuffer;
    nested_fb_t *old = current;

    while (true) {
        old = current;

        invalidate_whole_framebuffer_chain(current->child);
        invalidate_whole_framebuffer(current);

        current = current->next;
        if (old->next == NULL) break;
    }
}

void invalidate_partial_framebuffer(nested_fb_t *framebuffer, rectangle_t *dirty) {
    if (framebuffer->dirty.x1 == 0 &&
        framebuffer->dirty.y1 == 0 &&
        framebuffer->dirty.x2 == 0 &&
        framebuffer->dirty.y2 == 0)
    {
        framebuffer->dirty.x1 = dirty->x1;
        framebuffer->dirty.y1 = dirty->y1;
        framebuffer->dirty.x2 = dirty->x2;
        framebuffer->dirty.y2 = dirty->y2;
    } else {
        framebuffer->dirty.x1 = framebuffer->dirty.x1 < dirty->x1 ? framebuffer->dirty.x1 : dirty->x1;
        framebuffer->dirty.y1 = framebuffer->dirty.y1 < dirty->y1 ? framebuffer->dirty.y1 : dirty->y1;
        framebuffer->dirty.x2 = framebuffer->dirty.x2 > dirty->x2 ? framebuffer->dirty.x2 : dirty->x2;
        framebuffer->dirty.y2 = framebuffer->dirty.y2 > dirty->y2 ? framebuffer->dirty.y2 : dirty->y2;
    }
}

void render(nested_fb_t *source, nested_fb_t *target) {
    if (source == NULL || target == NULL) return;

    nested_fb_t *current = source;
    nested_fb_t *old = current;

    while (true) {
        old = current;

        render(current->child, current);
        blit_framebuffer_into_framebuffer(current, target);

        current = current->next;
        if (old->next == NULL) break;
    }
}

void blit_framebuffer_into_framebuffer(nested_fb_t *source, nested_fb_t *target) {
    uint32_t x = source->x;
    uint32_t y = source->y;
    uint32_t source_bytes_per_pixel = source->bpp / 8;
    uint32_t target_bytes_per_pixel = target->bpp / 8;

    uint32_t ymin = y + source->dirty.y1;
    uint32_t ymax = y + source->dirty.y2;
    uint32_t xmin = x + source->dirty.x1;
    uint32_t xmax = x + source->dirty.x2;
    ymax = ymax < target->height ? ymax : target->height;
    xmax = xmax < target->width ? xmax : target->width;
    if (ymin >= ymax) return;
    if (xmin >= xmax) return;

    for (int y1 = ymin; y1 < ymax; y1++) {
        for (int x1 = xmin; x1 < xmax; x1++) {
            size_t index_dst = (x1 + y1 * target->pitch / target_bytes_per_pixel) * target_bytes_per_pixel;
            size_t index_src = ((x1 - x) + (y1 - y) * source->pitch / source_bytes_per_pixel) * source_bytes_per_pixel;

            if (source->bpp == 32 && source->has_alpha && source->data[index_src + 3] == 0) continue;
            target->data[index_dst + 2] = source->data[index_src + 0];
            target->data[index_dst + 1] = source->data[index_src + 1];
            target->data[index_dst + 0] = source->data[index_src + 2];
        }
    }

    // if the target's dirty rectangle is not set, then set it directly to our coords and size
    if (target->dirty.x1 == 0 &&
        target->dirty.y1 == 0 &&
        target->dirty.x2 == 0 &&
        target->dirty.y2 == 0)
    {
        target->dirty.x1 = xmin;
        target->dirty.y1 = ymin;
        target->dirty.x2 = xmax;
        target->dirty.y2 = ymax;
    } else {
        // we already have a dirty rectangle, integrate our blit into it
        target->dirty.x1 = target->dirty.x1 < xmin ? target->dirty.x1 : xmin;
        target->dirty.y1 = target->dirty.y1 < ymin ? target->dirty.y1 : ymin;
        target->dirty.x2 = target->dirty.x2 > xmax ? target->dirty.x2 : xmax;
        target->dirty.y2 = target->dirty.y2 > ymax ? target->dirty.y2 : ymax;
    }

    // clear the source dirty rectangle
    source->dirty.x1 = 0;
    source->dirty.y1 = 0;
    source->dirty.x2 = 0;
    source->dirty.y2 = 0;
}
