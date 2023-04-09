#include <kernel/allocator.h>
#include <kernel/elf.h>
#include <kernel/framebuffer.h>
#include <kernel/process.h>

#include <fatfs/ff.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

process_context_t *scheduler_context;
process_t *processes[MAX_PROCESSES] = { 0 };
process_t *current_process = 0;

extern page_directory_t *current_page_directory;

static uint32_t find_unused_process() {
    for (uint8_t i = 0; i < MAX_PROCESSES; i++)
        if (!processes[i])
            return i;

    return (uint32_t) -1;
}

static void clean_up_process(uint32_t pid) {
    // TODO: free process memory here
    processes[pid]->state = UNUSED;
    // TODO: kfree(processes[pid]);
    processes[pid] = 0;
}

void init_scheduler() {
    scheduler_context = (process_context_t *) kallocate(sizeof(process_context_t), false, NULL);
}

void scheduler() {
    while (true) {
        for (uint8_t i = 0; i < MAX_PROCESSES; i++) {
            if (!processes[i]) continue;
            if (processes[i]->state == DEAD) {
                clean_up_process(i);
                continue;
            }
            if (processes[i]->state != RUNNABLE) continue;

            // mark the process as running and switch to it
            current_process = processes[i];
            current_process->state = RUNNING;
            scheduler_context->eip = (uint32_t) &&ret;
            page_directory_t *old_page_directory = current_page_directory;
            switch_page_directory(current_process->page_directory);
            switch_process(&scheduler_context, current_process->context);
ret:
            switch_page_directory(old_page_directory);

            // once we reach this point, the process has switched back here
            // if the process is still alive, mark it as RUNNABLE
            if (current_process->state != DEAD)
                current_process->state = RUNNABLE;
            current_process = 0;
        }
    }
}

bool new_process(char path[], char *argv[]) {
    uint32_t new_pid = find_unused_process();
    if (new_pid == (uint32_t) -1) {
        kprintf("max process limit reached, failed to create new process: %s\n", path);
        return false;
    }

    // open the file
    // TODO: use VFS once that is implemented
    FIL binary;
    FRESULT result = f_open(&binary, path, FA_READ);
    if (result != FR_OK) {
        kprintf("failed to open file for new process: %s\nerror: %d\n", path, result);
        return false;
    }
    uint32_t binary_size = f_size(&binary);

    // create a new page directory for this process
    page_directory_t *process_page_directory = (page_directory_t *) kallocate(sizeof(page_directory_t), true, NULL);
    memset(process_page_directory, 0, sizeof(page_directory_t));
    map_kernel(process_page_directory);
    map_framebuffer(process_page_directory);

    // map enough consecutive pages starting at the ELF start address
    uint8_t *binary_buffer = (uint8_t *) kallocate(binary_size, false, NULL);
    if (!binary_buffer) {
        kprintf("failed to allocate buffer for new process: %s\n", path);
        // TODO: kfree process_page_directory here
        f_close(&binary);
        return false;
    }

    // read the entire file into the buffer
    // TODO: use VFS once that is implemented
    unsigned int bytes_read;
    result = f_read(&binary, binary_buffer, binary_size, &bytes_read);
    if (result != FR_OK || bytes_read != binary_size) {
        kprintf("failed to read file for new process: %s\nerror: %d\n", path, result);
        // TODO: kfree process_page_directory here
        f_close(&binary);
        return false;
    }

    // allocate memory for the process's state
    process_t *process = (process_t *) kallocate(sizeof(process_t), false, NULL);
    if (!process) {
        kprintf("failed to allocate memory for new process state: %s\n", path);
        // TODO: kfree process_page_directory here
        f_close(&binary);
        return false;
    }
    uint8_t *process_stack_pointer = (uint8_t *) kallocate(65536, false, NULL);
    if (!process_stack_pointer) {
        kprintf("failed to allocate memory for new process stack: %s\n", path);
        // TODO: kfree process_page_directory here
        //       kfree process here
        f_close(&binary);
        return false;
    }

    // set the stack pointer
    process_stack_pointer += 65536 - sizeof(process_context_t);

    // parse the ELF binary and set EIP
    process->context = (process_context_t *) process_stack_pointer;
    process->context->eip = parse_elf(process_page_directory, binary_buffer);
    if (!process->context->eip) {
        kprintf("failed to parse and prepare the ELF for new process: %s\n", path);
        // TODO: kfree process_page_directory here
        //       kfree process here
        f_close(&binary);
        return false;
    }

    process->pid = new_pid;
    process->state = RUNNABLE;
    process->page_directory = process_page_directory;
    processes[new_pid] = process;

    return true;
}

void exit_process() {
    kprintf("exiting process %d\n", current_process->pid);
    current_process->state = DEAD;
    yield_process();
}

void yield_process() {
    switch_process(&current_process->context, scheduler_context);
}
