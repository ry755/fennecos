// adapted from http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html with some fixes applied

#include <kernel/allocator.h>
#include <kernel/paging.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

page_directory_t *kernel_page_directory;
page_directory_t *current_page_directory;

uint32_t *frames;
uint32_t number_of_frames;

#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

static uint32_t first_free_frame() {
    // starting at 256 to skip the first 1 MiB
    for (uint32_t i = 256; i < INDEX_FROM_BIT(number_of_frames); i++) {
        if (frames[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t to_test = 1 << j;
                if (!(frames[i] & to_test)) {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    return (uint32_t) -1;
}

void init_paging() {
    const uint32_t end_page = 0xFFFFFFFF;
    number_of_frames = end_page / 0x1000;
    frames = (uint32_t *) kallocate(INDEX_FROM_BIT(number_of_frames), false, NULL);
    memset(frames, 0, INDEX_FROM_BIT(number_of_frames));

    kernel_page_directory = (page_directory_t *) kallocate(sizeof(page_directory_t), true, NULL);
    memset(kernel_page_directory, 0, sizeof(page_directory_t));
    current_page_directory = kernel_page_directory;

    map_kernel(kernel_page_directory);
    switch_page_directory(kernel_page_directory);
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0": "=r" (cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0":: "r" (cr0));
}

void switch_page_directory(page_directory_t *page_directory) {
    current_page_directory = page_directory;
    __asm__ volatile("mov %0, %%cr3":: "r" (&page_directory->tables_physical));
}

void map_kernel(page_directory_t *page_directory) {
    // identity map 63 MiB starting at 1 MiB
    for (uint32_t i = 0x00100000; i < 0x04000000; i += 0x1000)
        map_physical_to_virtual(page_directory, i, i, true, true);
}

void map_physical_to_virtual(page_directory_t *page_directory, uint32_t physical_address, uint32_t virtual_address, bool is_kernel, bool is_writable) {
    allocate_frame(get_page(virtual_address, true, page_directory), physical_address, is_kernel, is_writable);
}

uint32_t map_consecutive(page_directory_t *page_directory, uint32_t pages_to_map, bool is_kernel, bool is_writable) {
    // first, we need to find enough consecutive free pages
    // search virtual addresses ranging from 4 KiB to 128 MiB
    //FIXME: this range shouldn't be hardcoded
    page_t *page;
    uint32_t virtual_address = 0x00001000;
    const uint32_t end_virtual_address = 0x08000000;
retry:
    for (; virtual_address < end_virtual_address; virtual_address += 0x1000) {
        page = get_page(virtual_address, false, page_directory);
        if (!page)
            break;
    }
    if (virtual_address >= end_virtual_address)
        return 0;

    // virtual_address now equals an unmapped virtual address
    // now, check if there are enough consecutive unmapped pages after it
    for (uint32_t i = virtual_address; i < virtual_address + (pages_to_map * 0x1000); i += 0x1000) {
        page = get_page(i, false, page_directory);
        if (page) {
            virtual_address += i - virtual_address;
            goto retry;
        }
    }

    // if we reached this point then there were enough consecutive pages!
    // now let's map them!
    for (uint32_t i = 0; i < pages_to_map; i++)
        map_physical_to_virtual(page_directory, 0, virtual_address + (i * 0x1000), is_kernel, is_writable);

    return virtual_address;
}

uint32_t map_consecutive_starting_at(page_directory_t *page_directory, uint32_t virtual_address, uint32_t pages_to_map, bool is_kernel, bool is_writable) {
    // check if there are enough consecutive unmapped pages after it
    for (uint32_t i = virtual_address; i < virtual_address + (pages_to_map * 0x1000); i += 0x1000) {
        page_t *page = get_page(i, false, page_directory);
        if (page)
            return 0;
    }

    // if we reached this point then there were enough consecutive pages!
    // now let's map them!
    for (uint32_t i = 0; i < pages_to_map; i++)
        map_physical_to_virtual(page_directory, 0, virtual_address + (i * 0x1000), is_kernel, is_writable);

    return virtual_address;
}

page_t *get_page(uint32_t virtual_address, bool make, page_directory_t *page_directory) {
    virtual_address /= 0x1000;
    uint32_t table_idx = virtual_address / 1024;
    if (page_directory->tables[table_idx]) {
        return &page_directory->tables[table_idx]->pages[virtual_address % 1024];
    } else if (make) {
        uint32_t tmp;
        page_directory->tables[table_idx] = (page_table_t *) kallocate(sizeof(page_table_t), true, &tmp);
        memset(page_directory->tables[table_idx], 0, 0x1000);
        page_directory->tables_physical[table_idx] = tmp | 0x7; // PRESENT, RW, USER
        return &page_directory->tables[table_idx]->pages[virtual_address % 1024];
    } else {
        return 0;
    }
}

void allocate_frame(page_t *page, uint32_t physical_address, bool is_kernel, bool is_writable) {
    if (page->frame != 0) {
        return;
    } else {
        uint32_t idx = first_free_frame();
        if (physical_address)
            idx = (physical_address & 0xFFFFF000) / 0x1000;
        if (idx == (uint32_t) -1) {
            kprintf("no free frames!\n");
            abort();
        }
        set_frame(idx * 0x1000);
        page->present = 1;
        page->rw = is_writable ? 1 : 0;
        page->user = is_kernel ? 0 : 1;
        page->frame = idx;
    }
}

void free_frame(page_t *page) {
    uint32_t frame;
    if (!(frame = page->frame)) {
        return;
    } else {
        clear_frame(frame);
        page->frame = 0;
    }
}
