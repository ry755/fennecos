#include <user/user.h>

void _start() {
    draw_string("test", 16, 32, 0xFFFFFFFF, 0xFF123456);
    while (1);
}
