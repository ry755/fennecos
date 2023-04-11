#pragma once

#include <kernel/paging.h>
#include <kernel/vfs.h>

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
    UNUSED   = 0,
    DEAD     = 1,
    SLEEPING = 2,
    RUNNABLE = 3,
    RUNNING  = 4
} process_state_t;

typedef struct process_s {
    page_directory_t *page_directory;
    process_context_t *context;
    process_state_t state;
    uint32_t pid;
    char current_directory[256];
    file_t *files[16];
} process_t;

void init_scheduler();
void scheduler();
bool new_process(char path[], char *argv[], file_t *stdin_file, file_t *stdout_file);
void exit_process();
void yield_process();
void switch_process(process_context_t **old, process_context_t *new);
