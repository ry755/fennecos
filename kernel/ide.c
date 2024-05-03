#include <kernel/ide.h>
#include <kernel/io.h>
#include <kernel/isr.h>
#include <kernel/timer.h>

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#include <stdint.h>

void init_ide() {
    install_interrupt_handler(14, ide_interrupt_handler);
}

void wait_for_disk() {
    // wait for the disk to become ready
    uint32_t start_tick = get_timer_value();
    while((inb(0x1F7) & 0xC0) != 0x40)
        if (get_timer_value() > start_tick + 512)
            return;
}

void read_sector(void *destination, uint32_t sector) {
    wait_for_disk();
    outb(0x1F2, 1);
    outb(0x1F3, sector);
    outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16);
    outb(0x1F6, (sector >> 24) | 0xE0);
    outb(0x1F7, 0x20);

    wait_for_disk();
    insl(0x1F0, destination, SECTOR_SIZE / 4);
}

void write_sector(const void *source, uint32_t sector) {
    wait_for_disk();
    outb(0x1F2, 1);
    outb(0x1F3, sector);
    outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16);
    outb(0x1F6, (sector >> 24) | 0xE0);
    outb(0x1F7, 0x30);

    wait_for_disk();
    outsl(0x1F0, source, SECTOR_SIZE / 4);
}

DRESULT ide_ioctl(unsigned char command, void *buffer) {
    (void) command;
    (void) buffer;
    return RES_NOTRDY;
}

void ide_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;
    // nothing
}
