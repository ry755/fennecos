#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct page_s {
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;
} page_t;

typedef struct page_table_s {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024]; // array of pointers to page tables
    uint32_t tables_physical[1024]; // array of pointers to the page tables above, but gives their *physical* location
    uint32_t physical_address; // the physical address of tables_physical
} page_directory_t;

void init_paging();
void switch_page_directory(page_directory_t *page_directory);
void map_physical_to_virtual(page_directory_t *page_directory, uint32_t physical_address, uint32_t virtual_address, bool is_kernel, bool is_writable);
page_t *get_page(uint32_t address, bool make, page_directory_t *page_directory);
void allocate_frame(page_t *page, uint32_t virtual_address, bool is_kernel, bool is_writable);
void free_frame(page_t *page);
