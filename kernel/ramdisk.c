#include <kernel/serial.h>
#include <kernel/ramdisk.h>
#include <kernel/vfs.h>

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uint8_t ramdisk[RAMDISK_SIZE];

void init_ramdisk(FATFS *disk, char image_path[]) {
    kprintf("mounting ramdisk\n");

    file_t ramdisk_file;
    uint32_t result = open(&ramdisk_file, image_path, MODE_READ);
    if (!result) {
        kprintf("failed to open %s\n", image_path);
        kprintf("error: %d\n", result);
        return;
    }

    uint32_t bytes_read = read(&ramdisk_file, (char *) ramdisk, RAMDISK_SIZE);
    if (bytes_read != RAMDISK_SIZE)
        kprintf("ramdisk image read short\n");

    close(&ramdisk_file);

    result = f_mount(disk, "0:", 1);
    if (result != FR_OK) {
        kprintf("failed to mount 0:\n");
        kprintf("error: %d\n", result);
    }
}

void read_ramdisk_sector(void *destination, uint32_t sector) {
    memcpy(destination, &ramdisk[sector * 512], 512);
}

void write_ramdisk_sector(const void *source, uint32_t sector) {
    memcpy(&ramdisk[sector * 512], source, 512);
}

DRESULT ramdisk_ioctl(unsigned char command, void *buffer) {
    switch (command) {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            *(LBA_t *) buffer = RAMDISK_SIZE / 512;
            return RES_OK;

        case GET_SECTOR_SIZE:
            *(uint32_t *) buffer = 512;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(uint32_t *) buffer = 1;
            return RES_OK;

        case CTRL_TRIM:
            return RES_OK;

        default:
            return RES_PARERR;
    }
}
