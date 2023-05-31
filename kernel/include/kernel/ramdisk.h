#pragma once

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#include <stdint.h>

#define RAMDISK_SIZE 8388608

void init_ramdisk();
void read_ramdisk_sector(void *destination, uint32_t sector);
void write_ramdisk_sector(const void *source, uint32_t sector);
DRESULT ramdisk_ioctl(unsigned char command, void *buffer);
