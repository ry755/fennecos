#include <user/user.h>
#include <user/framebuffer.h>
#include <user/keyboard.h>

event_t event;

void _start() {
    draw_string("test", 16, 16, 0xFFFFFFFF, 0xFF123456, global_font);
    while (1) {
        if (get_next_event(&event)) {
            switch (event.type) {
                case KEY_DOWN:
                    draw_string("key down", 16, 32, 0xFFFFFFFF, 0xFF123456, global_font);
                    keyboard_event(event.arg0);
                    draw_font_tile(scancode_to_ascii(event.arg0), 16, 48, 0xFFFFFFFF, 0xFF123456, global_font);
                    break;

                case KEY_UP:
                    draw_string("key up  ", 16, 32, 0xFFFFFFFF, 0xFF123456, global_font);
                    keyboard_event(event.arg0);
                    draw_font_tile(scancode_to_ascii(event.arg0 & 0x7F), 16, 48, 0xFFFFFFFF, 0xFF123456, global_font);
                    break;
            }
        }
        yield_process();
    }
    exit_process();
}
