#include <kernel/allocator.h>
#include <kernel/paging.h>

#include <stdbool.h>
#include <stdint.h>

block_header_t *free_list_head;
bool allocator_initialized = false;
uint32_t kalloc_before_init_ptr = 0x4000;

extern page_directory_t *kernel_page_directory;

void init_allocator() {
    // FIXME: these values shouldn't be hardcoded
    free_list_head = (block_header_t *) 0x01000000;
    free_list_head->size = 0x01000000;
    free_list_head->next = 0;
    free_list_head->prev = 0;
    allocator_initialized = true;
}

void *allocate(uint32_t size) {
    block_header_t *block = free_list_head;
    uint32_t real_size = size + sizeof(block_header_t);

    while (block) {
        if (block->size >= real_size) {
            if (block->size <= real_size + sizeof(block_header_t) + 32) {
                block_header_t *next = block->next;
                block_header_t *prev = block->prev;

                if (next)
                    next->prev = prev;

                if (prev)
                    prev->next = next;
                else
                    free_list_head = next;

                return block + sizeof(block_header_t);
            }

            block->size -= real_size;
            block = (block_header_t *)((uint8_t *)block + block->size);
            block->size = real_size;
            return block + sizeof(block_header_t);
        }

        block = block->next;
    }

    return 0;
}

void free(block_header_t *ptr) {
    ptr -= sizeof(block_header_t);

    ptr->prev = 0;
    ptr->next = free_list_head;

    if (free_list_head)
        free_list_head->prev = ptr;

    free_list_head = ptr;
}

uint32_t kallocate(uint32_t size, bool align, uint32_t *physical) {
    if (allocator_initialized) {
        if (align)
            size += 0x1000;
        uint32_t ptr = (uint32_t) allocate(size);
        if (align) {
            ptr &= 0xFFFFF000;
            ptr += 0x1000;
        }
        if (physical) {
            page_t *page = get_page((uint32_t) ptr, false, kernel_page_directory);
            *physical = page->frame * 0x1000 + ((uint32_t) ptr & 0x0FFF);
        }
        return ptr;
    } else {
        if (align && (kalloc_before_init_ptr & 0x00000FFF)) {
            kalloc_before_init_ptr &= 0xFFFFF000;
            kalloc_before_init_ptr += 0x1000;
        }
        if (physical)
            *physical = kalloc_before_init_ptr;
        uint32_t ptr = kalloc_before_init_ptr;
        kalloc_before_init_ptr += size;
        return ptr;
    }
}
