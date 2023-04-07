#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct block_header_s {
    uint32_t size;
    struct block_header_s *prev;
    struct block_header_s *next;
} block_header_t;

void init_allocator();
void *allocate(uint32_t size);
void free(block_header_t *ptr);
uint32_t kallocate(uint32_t size, bool align, uint32_t *physical);
