#pragma once

#include <kernel/paging.h>
#include <kernel/vfs.h>

#include <stdbool.h>
#include <stdint.h>

#define MAX_PROCESSES 32

#define SIGNAL_CHECK 0
#define SIGNAL_KILL  9

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
    uintptr_t stack_ptr_to_free;
    page_directory_t *page_directory;
    process_context_t *context;
    process_state_t state;
    uint32_t pid;
    char name[32];
    char current_directory[256];
    file_t *files[16];
    uint32_t sleep_until;
} process_t;

void init_scheduler();
void scheduler();
uint32_t new_process(char path[], char *argv[], file_t *stdin_file, file_t *stdout_file);
void exit_process();
bool kill_process(uint32_t pid, uint32_t signal);
void yield_process();
void switch_process(process_context_t **old, process_context_t *new);
void sleep_process(uint32_t ticks);
