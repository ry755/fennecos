#include <kernel/allocator.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

block_header_t *free_list_head;
bool allocator_initialized = false;

void init_allocator() {
    // FIXME: these values shouldn't be hardcoded
    free_list_head = (block_header_t *) 0x01000000;
    free_list_head->size = 0x02000000;
    free_list_head->next = 0;
    free_list_head->prev = 0;
    free_list_head->actual_starting_address = 0;
    free_list_head->flag = BLOCK_FREE_FLAG;
    allocator_initialized = true;
}

void *allocate(uint32_t size, bool align) {
    if (!allocator_initialized) {
        kprintf("attempting to allocate before allocator initialized!\n");
        abort();
    }

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
                block->flag = BLOCK_USED_FLAG;
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
            block->flag = BLOCK_USED_FLAG;
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

    // ensure the block header is okay and this isn't a double-free
    if ((bptr->flag != BLOCK_FREE_FLAG) && (bptr->flag != BLOCK_USED_FLAG)) {
        kprintf("BLOCK AT 0x%x APPEARS CORRUPTED!!\n", (uint32_t) ptr);
        kprintf(
            "block info\nbptr->size = 0x%x\nbptr->prev = 0x%x\nbptr->next = 0x%x\nbptr->actual_starting_address = 0x%x\nbptr->flag = 0x%x\n",
            bptr->size, bptr->prev, bptr->next, bptr->actual_starting_address, bptr->flag
        );
        abort();
    } else if (bptr->flag == BLOCK_FREE_FLAG) {
        kprintf("BLOCK AT 0x%x ALREADY FREED!!\n", (uint32_t) ptr);
        kprintf(
            "block info\nbptr->size = 0x%x\nbptr->prev = 0x%x\nbptr->next = 0x%x\nbptr->actual_starting_address = 0x%x\nbptr->flag = 0x%x\n",
            bptr->size, bptr->prev, bptr->next, bptr->actual_starting_address, bptr->flag
        );
        abort();
    }

    bptr->prev = 0;
    bptr->next = free_list_head;
    bptr->flag = BLOCK_FREE_FLAG;

    if (free_list_head)
        free_list_head->prev = bptr;

    free_list_head = bptr;
}
