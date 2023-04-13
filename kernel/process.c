#include <kernel/allocator.h>
#include <kernel/elf.h>
#include <kernel/framebuffer.h>
#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/vfs.h>

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
    bool flag;
    do {
        flag = false;
        for (uint8_t i = 0; i < MAX_PROCESSES; i++) {
            if (!processes[i]) continue;
            flag = true;
            if (processes[i]->state == DEAD) {
                clean_up_process(i);
                continue;
            }
            if (processes[i]->state == SLEEPING)
                if (get_timer_value() >= processes[i]->sleep_until)
                    processes[i]->state = RUNNABLE;
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
    } while (flag);
}

bool new_process(char path[], char *argv[], file_t *stdin_file, file_t *stdout_file) {
    uint32_t new_pid = find_unused_process();
    if (new_pid == (uint32_t) -1) {
        kprintf("max process limit reached, failed to create new process: %s\n", path);
        return false;
    }

    // open the file
    file_t binary;
    if (!open(&binary, path, MODE_READ)) {
        kprintf("failed to open file for new process: %s\n", path);
        return false;
    }
    uint32_t binary_size = f_size(&binary.fatfs); // TODO: implement size getting function in the VFS

    // create a new page directory for this process
    page_directory_t *process_page_directory = (page_directory_t *) kallocate(sizeof(page_directory_t), true, NULL);
    memset(process_page_directory, 0, sizeof(page_directory_t));
    map_kernel(process_page_directory);
    map_framebuffer(process_page_directory);

    // allocate a buffer to hold the file
    uint8_t *binary_buffer = (uint8_t *) kallocate(binary_size, false, NULL);
    if (!binary_buffer) {
        kprintf("failed to allocate buffer for new process: %s\n", path);
        // TODO: kfree process_page_directory here
        close(&binary);
        return false;
    }

    // read the entire file into the buffer
    uint32_t bytes_read = read(&binary, (char *) binary_buffer, binary_size);
    if (bytes_read != binary_size) {
        kprintf("failed to read file for new process: %s, error: %d\n", path);
        // TODO: kfree process_page_directory here
        close(&binary);
        return false;
    }

    // allocate memory for the process's state
    process_t *process = (process_t *) kallocate(sizeof(process_t), false, NULL);
    if (!process) {
        kprintf("failed to allocate memory for new process state: %s\n", path);
        // TODO: kfree process_page_directory here
        close(&binary);
        return false;
    }
    uint8_t *process_stack_pointer = (uint8_t *) kallocate(65536, false, NULL);
    if (!process_stack_pointer) {
        kprintf("failed to allocate memory for new process stack: %s\n", path);
        // TODO: kfree process_page_directory here
        //       kfree process here
        close(&binary);
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
        close(&binary);
        return false;
    }

    // push argument strings to the stack
    uint32_t argc = 0;
    if (argv) {
        for (argc = 0; argv[argc]; argc++) {
            // TODO: implement
        }
    }

    process->pid = new_pid;
    process->state = RUNNABLE;
    process->page_directory = process_page_directory;
    strcpy(process->current_directory, strip_last_path_component(path));

    // if there is a current process, make the new process inherit the current process's stdio streams
    if (current_process) {
        process->files[0] = current_process->files[0];
        process->files[1] = current_process->files[1];
    } else if (stdin_file || stdout_file) {
        process->files[0] = stdin_file;
        process->files[1] = stdout_file;
    }

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

void sleep_process(uint32_t ticks) {
    uint32_t tick = get_timer_value();
    if (current_process) {
        // if there is a current process, make it sleep
        current_process->sleep_until = tick + ticks;
        current_process->state = SLEEPING;
        yield_process();
    } else {
        // otherwise we're in the kernel, just delay. this is bad but whatever
        while (get_timer_value() < tick + ticks);
    }
}
