#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "window.h"

void gfx_move_to(uint16_t x, uint16_t y);
void gfx_set_port(window_t *window);
void gfx_draw_font_tile(char tile);
void gfx_draw_string(char *str);
