// based on information from here: https://wiki.osdev.org/Programmable_Interval_Timer

#include <kernel/io.h>
#include <kernel/pit.h>

#include <stdint.h>

void init_pit() {
    // mode 2
    outb(0x43, 0x36);

    // 100 Hz
    outb(0x40, 11931 & 0x00FF);
    outb(0x40, (11931 & 0xFF00) >> 8);
}
