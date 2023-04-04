#pragma once

#include <stdint.h>

#define SECTOR_SIZE 512

void wait_for_disk();
void read_sector(void *destination, uint32_t sector);
void write_sector(void *source, uint32_t sector);
