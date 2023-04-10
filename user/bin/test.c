#include <user/user.h>
#include <user/framebuffer.h>
#include <user/keyboard.h>

#include <stdbool.h>

event_t event;
file_t file;
bool running = true;

void _start() {
    draw_string("demo - press ESC to exit", 16, 16, 0xFFFFFFFF, 0xFF123456, global_font);

    char file_buffer[512] = { 0 };
    if (open(&file, "1:/res/hello.txt", MODE_READ)) {
        read(&file, file_buffer, 512);
        draw_string(file_buffer, 16, 32, 0xFFFFFFFF, 0xFF123456, global_font);
    } else {
        draw_string("failed to open 1:/res/hello.txt", 16, 32, 0xFFFFFFFF, 0xFF123456, global_font);
    }

    while (running) {
        if (get_next_event(&event)) {
            switch (event.type) {
                case KEY_DOWN:
                    draw_string("key down", 16, 48, 0xFFFFFFFF, 0xFF123456, global_font);
                    keyboard_event(event.arg0);
                    draw_font_tile(scancode_to_ascii(event.arg0), 16, 64, 0xFFFFFFFF, 0xFF123456, global_font);
                    break;

                case KEY_UP:
                    draw_string("key up  ", 16, 48, 0xFFFFFFFF, 0xFF123456, global_font);
                    keyboard_event(event.arg0);
                    draw_font_tile(scancode_to_ascii(event.arg0 & 0x7F), 16, 64, 0xFFFFFFFF, 0xFF123456, global_font);
                    if (event.arg0 == 0x81)
                        running = false;
                    break;
            }
        }
        yield();
    }
    exit();
}
