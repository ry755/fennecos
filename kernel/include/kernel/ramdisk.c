#include <kernel/serial.h>
#include <kernel/ramdisk.h>

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ramdisk_mbr.h"

uint8_t ramdisk_mkfs_work[512];
MKFS_PARM ramdisk_mkfs_parm = {
    .fmt = FM_FAT32,
    .n_fat = 2,
    .align = 0,
    .n_root = 0,
    .au_size = 0
};

void init_ramdisk() {
    FRESULT result = f_mkfs("0:", &ramdisk_mkfs_parm, ramdisk_mkfs_work, 512);
    if (result != FR_OK) {
        write_serial_string("failed to format 0:\n");
        printf("error: %d\n", result);
        abort();
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
