#include <kernel/allocator.h>
#include <kernel/paging.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

extern uint32_t _kernel_end;
extern page_directory_t *kernel_page_directory;

block_header_t *free_list_head;
bool allocator_initialized = false;
uintptr_t kalloc_before_init_ptr = (uintptr_t) &_kernel_end;

void init_allocator() {
    // FIXME: these values shouldn't be hardcoded
    free_list_head = (block_header_t *) 0x01000000;
    free_list_head->size = 0x05000000;
    free_list_head->next = 0;
    free_list_head->prev = 0;
    allocator_initialized = true;
}

void *allocate(uint32_t size, bool align) {
    block_header_t *block = free_list_head;
    uint32_t real_size = size + sizeof(block_header_t);
    if (align) real_size += 0x1000;

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

                block->actual_starting_address = 0;
                if (align) {
                    // if we need to be aligned, set the address to the start of the actual block and round up the returned block address
                    block_header_t *old_block = block;
                    block = (block_header_t *) ((uint32_t) block & 0xFFFFF000);
                    block = (block_header_t *) ((uint32_t) block + 0x1000 - sizeof(block_header_t));
                    block->actual_starting_address = (uint32_t) old_block;
                }

                block_header_t *final_block = (block_header_t *) ((uint8_t *) block + sizeof(block_header_t));
                kprintf("allocated block at 0x%x, align: %s\n", (uint32_t) final_block, align ? "true" : "false");
                return final_block;
            }

            block->size -= real_size;

            // new block
            block = (block_header_t *) ((uint8_t *) block + block->size);
            block->size = real_size;
            block->actual_starting_address = 0;
            if (align) {
                // if we need to be aligned, set the address to the start of the actual block and round up the returned block address
                block_header_t *old_block = block;
                block = (block_header_t *) ((uint32_t) block & 0xFFFFF000);
                block = (block_header_t *) ((uint32_t) block + 0x1000 - sizeof(block_header_t));
                block->actual_starting_address = (uint32_t) old_block;
            }

            block_header_t *final_block = (block_header_t *) ((uint8_t *) block + sizeof(block_header_t));
            kprintf("allocated block at 0x%x, align: %s\n", (uint32_t) final_block, align ? "true" : "false");
            return final_block;
        }

        block = block->next;
    }

    return 0;
}

void free(void *ptr) {
    block_header_t *bptr = (block_header_t *) ((uint8_t *) ptr - sizeof(block_header_t));

    kprintf("freeing block at 0x%x\n", (uint32_t) ptr);

    // if actual_starting_address is not zero then this was an aligned block
    if (bptr->actual_starting_address != 0) {
        bptr = (block_header_t *) bptr->actual_starting_address;
        kprintf("block appeared to be aligned, block actually begins at 0x%x\n", (uint32_t) bptr);
    }

    bptr->prev = 0;
    bptr->next = free_list_head;

    if (free_list_head)
        free_list_head->prev = bptr;

    free_list_head = bptr;
}

uint32_t kallocate(uint32_t size, bool align, uint32_t *physical) {
    if (allocator_initialized) {
        uint32_t ptr = (uint32_t) allocate(size, align);
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
