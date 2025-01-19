#include <user/keyboard.h>

#include <stdbool.h>
#include <stdint.h>

char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', // Tab
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, // Control
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',
    0,  // Alt
    ' ', // Space
    0, // Caps lock
    0, // F1
    0, 0, 0, 0, 0, 0, 0, 0,
    0, // F10
    0, // Num lock
    0, // Scroll lock
    0, // Home
    0, // Up arrow
    0, // Page Up
    '-',
    0, // Left arrow
    0,
    0, // Right arrow
    '+',
    0, // End
    0, // Down arrow
    0, // Page Down
    0, // Insert
    0, // Delete
    0, 0, 0,
    0, // F11
    0, // F12
    0, // All other keys are undefined
};

char scancode_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', // Tab
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, // Control
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',
    0,  // Alt
    ' ', // Space
    0, // Caps lock
    0, // F1
    0, 0, 0, 0, 0, 0, 0, 0,
    0, // F10
    0, // Num lock
    0, // Scroll lock
    0, // Home
    0, // Up arrow
    0, // Page Up
    '-',
    0, // Left arrow
    0,
    0, // Right arrow
    '+',
    0, // End
    0, // Down arrow
    0, // Page Down
    0, // Insert
    0, // Delete
    0, 0, 0,
    0, // F11
    0, // F12
    0, // All other keys are undefined
};

bool shift = false;
bool ctrl = false;

bool keyboard_event(uint8_t scancode) {
    if (scancode == LSHIFT_PRESS || scancode == RSHIFT_PRESS) {
        shift = true;
        return false;
    } else if (scancode == LSHIFT_RELEASE || scancode == RSHIFT_RELEASE) {
        shift = false;
        return false;
    } else if (scancode == LCTRL_PRESS) {
        ctrl = true;
        return false;
    } else if (scancode == LCTRL_RELEASE) {
        ctrl = false;
        return false;
    }

    return true;
}

char scancode_to_ascii(uint8_t scancode) {
    if (shift)
        return scancode_map_shift[scancode];
    else if (ctrl)
        return scancode ? scancode_map_shift[scancode] - '@' : 0;
    else
        return scancode_map[scancode];
}
