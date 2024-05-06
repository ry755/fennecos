// based on information from here: http://www.brokenthorn.com/Resources/OSDevPic.html

#include <kernel/io.h>
#include <kernel/pic.h>

#include <stdint.h>

void init_pic() {
    // setup ICW 1
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // setup ICW 2
    outb(0x21, 0x20); // IRQ 0 mapped to interrupt 0x20
    outb(0xA1, 0x28); // IRQ 8 mapped to interrupt 0x28

    // setup ICW 3
    outb(0x21, 0x04); // use IRQ 2 to communicate with the secondary PIC
    outb(0xA1, 0x02);

    // setup ICW 4
    outb(0x21, 0x01); // enable x86 mode
    outb(0xA1, 0x01);

    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void pic_unmask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }
    outb(port, inb(port) & ~(1 << irq));
}

void end_of_interrupt(uint8_t irq) {
    if (irq >= 8)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
