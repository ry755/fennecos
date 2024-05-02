#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct block_header_s {
    uint32_t size;
    struct block_header_s *prev;
    struct block_header_s *next;
} block_header_t;

void *malloc(size_t size);
void free(const void *ptr);
