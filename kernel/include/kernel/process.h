#pragma once

#include <kernel/paging.h>

#include <stdbool.h>
#include <stdint.h>

#define MAX_PROCESSES 32

typedef struct process_context_s {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} process_context_t;

typedef enum process_state_e {
    DEAD     = 0,
    SLEEPING = 1,
    RUNNABLE = 2,
    RUNNING  = 3
} process_state_t;

typedef struct process_s {
    page_directory_t *page_directory;
    process_context_t *context;
    process_state_t state;
    uint32_t pid;
    char name[16];
} process_t;

void init_scheduler();
void scheduler();
bool new_process(char path[], char *argv[]);
void yield_process();
void switch_process(process_context_t **old, process_context_t *new);
