#include <stdint.h>
#include <string.h>

#include <fox/alloc.h>

static char malloc_buffer[131072];
static block_header_t *free_list_head = (block_header_t *) malloc_buffer;
static bool allocator_initialized = false;

void *malloc(size_t size) {
    if (!allocator_initialized) {
        free_list_head->size = 131072 - sizeof(block_header_t);
        free_list_head->next = 0;
        free_list_head->prev = 0;
        allocator_initialized = true;
    }

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

                block_header_t *final_block = (block_header_t *) ((uint8_t *) block + sizeof(block_header_t));
                return final_block;
            }

            block->size -= real_size;

            // new block
            block = (block_header_t *) ((uint8_t *) block + block->size);
            block->size = real_size;
            block_header_t *final_block = (block_header_t *) ((uint8_t *) block + sizeof(block_header_t));
            return final_block;
        }

        block = block->next;
    }

    return 0;
}
void free(const void *ptr) {
    block_header_t *bptr = (block_header_t *) ((uint8_t *) ptr - sizeof(block_header_t));

    bptr->prev = 0;
    bptr->next = free_list_head;

    if (free_list_head)
        free_list_head->prev = bptr;

    free_list_head = bptr;
}
