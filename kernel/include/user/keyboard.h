#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LSHIFT_PRESS   0x2A
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_PRESS   0x36
#define RSHIFT_RELEASE 0xB6
#define LCTRL_PRESS    0x1D
#define LCTRL_RELEASE  0x9D

bool keyboard_event(uint8_t scancode);
char scancode_to_ascii(uint8_t scancode);
