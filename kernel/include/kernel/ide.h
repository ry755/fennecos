#pragma once

#include <fatfs/ff.h>
#include <fatfs/diskio.h>

#include <stdint.h>

#define SECTOR_SIZE 512

void init_ide();
void wait_for_disk();
void read_sector(void *destination, uint32_t sector);
void write_sector(const void *source, uint32_t sector);
DRESULT ide_ioctl(unsigned char command, void *buffer);
void ide_interrupt_handler();
