#include <kernel/allocator.h>
#include <kernel/multiboot.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

block_header_t *free_list_head;
bool allocator_initialized = false;

extern uint32_t _kernel_end;

void init_allocator(uint32_t mmap_addr, uint32_t mmap_length) {
    kprintf("init_allocator(mmap_addr = 0x%X, mmap_length = 0x%X)\n", mmap_addr, mmap_length);
    multiboot_mmap_entry_t *mmap;

    mmap = (multiboot_mmap_entry_t *) mmap_addr;

    block_header_t *block = (block_header_t *)(uintptr_t) mmap->addr;
    free_list_head = block;
    uint32_t block_len;
    block_header_t *old_block = NULL;
    for (mmap = (multiboot_mmap_entry_t *) mmap_addr;
        (unsigned long) mmap < mmap_addr + mmap_length;
        mmap = (multiboot_mmap_entry_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)))
    {
        kprintf("    size = 0x%X, base_addr = 0x%X,"
            " length = 0x%X, type = 0x%X\n",
            (unsigned) mmap->size,
            (unsigned) (mmap->addr & 0xffffffff),
            (unsigned) (mmap->len & 0xffffffff),
            (unsigned) mmap->type
        );

        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE && mmap->addr != 0) {
            if (mmap->addr > (uintptr_t) &_kernel_end) {
                block = (block_header_t *)(uintptr_t) mmap->addr;
                block_len = mmap->len;
            } else {
                block = (block_header_t *) &_kernel_end;
                free_list_head = block;
                block_len = mmap->len - (uintptr_t) &_kernel_end;
            }
            kprintf("adding block: addr = 0x%X, size = 0x%X\n", (uint32_t) block, block_len);
            block->size = block_len;
            block->next = 0;
            block->prev = old_block;
            block->actual_starting_address = 0;
            block->flag = BLOCK_FREE_FLAG;
            if (old_block) old_block->next = block;
            old_block = block;
        }
    }

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
