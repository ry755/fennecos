#include <kernel/allocator.h>
#include <kernel/app.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t relocate_app(uint8_t *buffer) {
    app_header_t *header = (app_header_t *) buffer;
    if (!header) return false;

    kprintf(
        "relocate_app(): buffer = 0x%X, reloc_size = %d, reloc_offset = 0x%X, code_offset = 0x%X\n",
        (uint32_t) buffer,
        header->reloc_size,
        header->reloc_offset,
        header->code_offset
    );

    // check for the magic bytes
    if (header->magic != APP_MAGIC) {
        kprintf("failed to verify APP magic bytes, got 0x%X\n", header->magic);
        return 0;
    }

    for (uint32_t i = 0; i < header->reloc_size; i++) {
        // add the base address of the executable to the bytes that need relocating
        uint32_t reloc_offset = *((uint32_t *)buffer + header->reloc_offset + i);
        kprintf("relocating code at 0x%X\n", reloc_offset);
        *((uint32_t *)buffer + reloc_offset) += (uint32_t) buffer;
    }

    return (uint32_t) buffer + header->code_offset;
}
