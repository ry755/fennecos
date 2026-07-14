#include <user/user.h>
#include <user/framebuffer.h>

#include "bg.h"

#include <stdio.h>
#include <string.h>

void main(int argc, char *argv[]) {
    fb_t *framebuffer = (fb_t *) get_fb();

    for (int fb_i = 0, bg_i = 0; bg_i < bg_bitmap_len; bg_i += 3, fb_i += 4) {
        framebuffer->framebuffer[fb_i]     = bg_bitmap[bg_i + 2];
        framebuffer->framebuffer[fb_i + 1] = bg_bitmap[bg_i + 1];
        framebuffer->framebuffer[fb_i + 2] = bg_bitmap[bg_i];
    }

    printf("\e[31mHello world!\n");
    printf("\e[36mThis is a bitmap demo!\n");
    printf("\e[0m");
}
