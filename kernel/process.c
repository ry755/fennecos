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

#define APP_HEADER_SIZE     40

#define PROCESS_VADDR       0x07FFF000
#define PROCESS_EXTRA_PAGES 1
#define PROCESS_ARGS_VADDR  (PROCESS_VADDR)
#define PROCESS_CODE_VADDR  (PROCESS_ARGS_VADDR + 0x1000)

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

// TODO: free all of the pages in the process's page directory!!!!
static void clean_up_process(uint32_t pid) {
    processes[pid]->state = UNUSED;
    free((void *) processes[pid]->stack_ptr_to_free);
    free(processes[pid]->page_directory);
    free(processes[pid]);
    processes[pid] = 0;
}

void init_scheduler() {
    scheduler_context = (process_context_t *) kallocate(sizeof(process_context_t), false, NULL);
    current_process = NULL;
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

uint32_t new_process(char path[], char *argv[], file_t *stdin_file, file_t *stdout_file) {
    uint32_t new_pid = find_unused_process();
    if (new_pid == (uint32_t) -1) {
        kprintf("max process limit reached, failed to create new process: %s\n", path);
        return 0;
    }

    // open the file
    file_t binary;
    if (!open(&binary, path, MODE_READ)) {
        kprintf("failed to open file for new process: %s\n", path);
        return 0;
    }
    uint32_t binary_size = f_size(&binary.fatfs); // TODO: implement size getting function in the VFS

    uint32_t app_header[2];
    if (read(&binary, (char *) app_header, 8) != 8) {
        kprintf("failed to read APP header!\n");
        close(&binary);
        return 0;
    }
    if (app_header[0] != 0x00505041) {
        kprintf("failed to verify APP magic bytes!\n");
        close(&binary);
        return 0;
    }

    // the app is good, add the bss size to the total
    uint32_t total_size = binary_size + app_header[1];
    kprintf("total app size: %d\n", total_size);

    // create a new page directory for this process
    page_directory_t *process_page_directory = (page_directory_t *) kallocate(sizeof(page_directory_t), true, NULL);
    memset(process_page_directory, 0, sizeof(page_directory_t));
    map_kernel(process_page_directory);
    map_framebuffer(process_page_directory);

    // allocate a buffer to hold the file
    uint8_t *binary_buffer = (uint8_t *) kallocate(binary_size, false, NULL);
    if (!binary_buffer) {
        kprintf("failed to allocate buffer for new process: %s\n", path);
        free(process_page_directory);
        close(&binary);
        return 0;
    }

    // read the executable into the buffer
    seek(&binary, APP_HEADER_SIZE);
    uint32_t executable_size = binary_size - APP_HEADER_SIZE;
    uint32_t bytes_read = read(&binary, (char *) binary_buffer, executable_size);
    if (bytes_read != executable_size) {
        kprintf("failed to read file for new process: %s\n", path);
        free(binary_buffer);
        free(process_page_directory);
        close(&binary);
        return 0;
    }

    // allocate memory for the process's state
    process_t *process = (process_t *) kallocate(sizeof(process_t), false, NULL);
    if (!process) {
        kprintf("failed to allocate memory for new process state: %s\n", path);
        free(binary_buffer);
        free(process_page_directory);
        close(&binary);
        return 0;
    }

    // initialize it all to zero
    memset(process, 0, sizeof(process_t));

    uint8_t *process_stack_pointer = (uint8_t *) kallocate(65536, false, NULL);
    if (!process_stack_pointer) {
        kprintf("failed to allocate memory for new process stack: %s\n", path);
        free(binary_buffer);
        free(process_page_directory);
        free(process);
        close(&binary);
        return 0;
    }

    // set the pointers to the buffers to free when the process ends
    process->stack_ptr_to_free = (uintptr_t) process_stack_pointer;

    // set the initial stack pointer
    process_stack_pointer += 65535;

    uint32_t pages_needed = (total_size / 0x1000) + (PROCESS_EXTRA_PAGES + 1);
    kprintf("mapping %d pages starting at virtual address 0x%x for new process\n", pages_needed, PROCESS_VADDR);
    if (!map_consecutive_starting_at(process_page_directory, PROCESS_VADDR, pages_needed, true, true)) {
        kprintf("failed to map\n");
        free(binary_buffer);
        free((void *) process->stack_ptr_to_free);
        free(process_page_directory);
        free(process);
        close(&binary);
        return 0;
    }

    // put the argument strings in temporary memory
    // FIXME: this should really do length checks!
    //        i wrote this all at 5 AM and it really shows
    char *temp_args = NULL;
    uint32_t argc = 0;
    if (argv) {
        temp_args = (char *) kallocate(0x1000, false, NULL);
        if (temp_args) {
            while (argv[argc]) argc++;
            for (uint32_t i = 0; i < argc; i++)
                strcpy(&temp_args[i * 128], argv[i]);
        } else {
            kprintf("failed to allocate temporary buffer for arguments\n");
        }
    }

    // access new process memory so we can copy the args and the binary
    page_directory_t *old_page_directory = current_page_directory;
    switch_page_directory(process_page_directory);

    // this sucks and is probably wrong in a lot of ways but it Works who cares im going to sleep
    if (temp_args) {
        process_stack_pointer -= (argc * 4) + 12;
        ((uint32_t *)process_stack_pointer)[argc] = 0;
        for (uint32_t i = 0; i < argc; i++) {
            char *offset = (char *) PROCESS_ARGS_VADDR + (i * 128);
            strcpy(offset, &temp_args[i * 128]);
            offset[127] = '\0';
            ((uint32_t *)process_stack_pointer)[i + 2] = (uint32_t) offset;
        }
        ((uint32_t *)process_stack_pointer)[1] = (uint32_t) process_stack_pointer + 8;
        ((uint32_t *)process_stack_pointer)[0] = argc;
        free(temp_args);
    }

    // set the final stack pointer
    process_stack_pointer -= sizeof(process_context_t);

    // set up the initial stack and instruction pointers
    process->context = (process_context_t *) process_stack_pointer;
    process->context->eip = PROCESS_CODE_VADDR;

    // zero the memory, copy the binary, go back to the old page directory, and free the buffer
    memset((void *) PROCESS_CODE_VADDR, 0, total_size);
    memcpy((void *) PROCESS_CODE_VADDR, binary_buffer, executable_size);
    switch_page_directory(old_page_directory);
    free(binary_buffer);

    process->pid = new_pid;
    process->state = RUNNABLE;
    process->page_directory = process_page_directory;
    if (current_process) {
        strcpy(process->current_directory, current_process->current_directory);
    } else {
        char full_path[256];
        strcpy(process->current_directory, strip_last_path_component(make_absolute_path(path, full_path)));
    }

    // if there is a current process, make the new process inherit the current process's stdio streams
    if (current_process) {
        process->files[0] = current_process->files[0];
        process->files[1] = current_process->files[1];
    } else if (stdin_file || stdout_file) {
        process->files[0] = stdin_file;
        process->files[1] = stdout_file;
    }

    processes[new_pid] = process;
    return new_pid;
}

void exit_process() {
    kprintf("exiting process %d\n", current_process->pid);
    current_process->state = DEAD;
    yield_process();
}

bool kill_process(uint32_t pid, uint32_t signal) {
    if (processes[pid] == 0) return false;
    switch (signal) {
        case SIGNAL_CHECK: break;
        case SIGNAL_KILL:
            kprintf("killing process %d\n", pid);
            processes[pid]->state = DEAD;
            yield_process();
            break;
        default: return false;
    }
    return true;
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
