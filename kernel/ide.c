#include <kernel/ide.h>
#include <kernel/io.h>

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#include <stdint.h>

uint8_t temporary_sector_buffer[SECTOR_SIZE];

void wait_for_disk() {
    // wait for the disk to become ready
    while((inb(0x1F7) & 0xC0) != 0x40);
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