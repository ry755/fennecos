#pragma once

#include <kernel/process.h>

#include <stdint.h>

#define APP_MAGIC 0x00505041U

uint32_t relocate_app(uint8_t *buffer);

typedef struct app_header_s {
    uint32_t magic;
    uint32_t code_size;
    uint32_t code_offset;
    uint32_t reloc_size;
    uint32_t reloc_offset;
} app_header_t;
