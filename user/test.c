#include <user/user.h>

event_t event;

void _start() {
    draw_string("test", 16, 32, 0xFFFFFFFF, 0xFF123456);
    while (1) {
        if (get_next_event(&event)) {
            switch (event.type) {
                case KEY_DOWN:
                    draw_string("key down", 16, 48, 0xFFFFFFFF, 0xFF123456);
                    break;

                case KEY_UP:
                    draw_string("key up  ", 16, 48, 0xFFFFFFFF, 0xFF123456);
                    break;
            }
        }
        yield_process();
    }
    exit_process();
}
