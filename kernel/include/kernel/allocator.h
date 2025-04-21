#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BLOCK_USED_FLAG 0xA5A5A5A5
#define BLOCK_FREE_FLAG 0x1BADF00D

typedef struct block_header_s {
    uint32_t size;
    struct block_header_s *prev;
    struct block_header_s *next;
    uint32_t actual_starting_address; // used when allocating page-aligned block, otherwise zero
    uint32_t flag;  // safety against double frees: BLOCK_FREE_FLAG if free, BLOCK_USED_FLAG if used.
                    // if not either, assume corruption
} block_header_t;

void init_allocator();
void *allocate(uint32_t size, bool aligned);
void free(void *ptr);
uint32_t kallocate(uint32_t size, bool align, uint32_t *physical);
