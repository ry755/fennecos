#include <kernel/allocator.h>
#include <kernel/app.h>
#include <kernel/framebuffer.h>
#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/vfs.h>

#include <fatfs/ff.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// args are stored at offset 0, with the code being stored at +0x1000
#define PROCESS_CODE_OFFSET 0x1000

process_context_t *scheduler_context;
process_t *processes[MAX_PROCESSES] = { 0 };
process_t *current_process = 0;

static uint32_t find_unused_process() {
    for (uint8_t i = 0; i < MAX_PROCESSES; i++)
        if (!processes[i])
            return i;

    return (uint32_t) -1;
}

static void clean_up_process(uint32_t pid) {
    processes[pid]->state = UNUSED;
    free((void *) processes[pid]->code_ptr_to_free);
    free((void *) processes[pid]->stack_ptr_to_free);
    free(processes[pid]);
    processes[pid] = 0;
}

void init_scheduler() {
    scheduler_context = (process_context_t *) allocate(sizeof(process_context_t), false);
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
            switch_process(&scheduler_context, current_process->context);
ret:
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

    // allocate a buffer to hold the file
    uint8_t *binary_buffer = (uint8_t *) allocate(binary_size + PROCESS_CODE_OFFSET, false);
    if (!binary_buffer) {
        kprintf("failed to allocate buffer for new process: %s\n", path);
        close(&binary);
        return 0;
    }

    // read the executable into the buffer
    uint32_t bytes_read = read(&binary, (char *) binary_buffer + PROCESS_CODE_OFFSET, binary_size);
    if (bytes_read != binary_size) {
        kprintf("failed to read file for new process: %s\n", path);
        free(binary_buffer);
        close(&binary);
        return 0;
    }

    // allocate memory for the process's state
    process_t *process = (process_t *) allocate(sizeof(process_t), false);
    if (!process) {
        kprintf("failed to allocate memory for new process state: %s\n", path);
        free(binary_buffer);
        close(&binary);
        return 0;
    }

    // initialize it all to zero
    memset(process, 0, sizeof(process_t));

    uint8_t *process_stack_pointer = (uint8_t *) allocate(65536, false);
    if (!process_stack_pointer) {
        kprintf("failed to allocate memory for new process stack: %s\n", path);
        free(binary_buffer);
        free(process);
        close(&binary);
        return 0;
    }

    // set the pointers to the buffers to free when the process ends
    process->code_ptr_to_free = (uintptr_t) binary_buffer;
    process->stack_ptr_to_free = (uintptr_t) process_stack_pointer;

    // set the initial stack pointer
    process_stack_pointer += 65535;

    // FIXME: this should really do length checks!
    uint32_t argc = 0;
    if (argv) {
        while (argv[argc]) argc++;
        for (uint32_t i = 0; i < argc; i++) {
            kprintf("writing \"%s\" to 0x%X\n", argv[i], &binary_buffer[i * 128]);
            strcpy((char *)&binary_buffer[i * 128], argv[i]);
        }

        process_stack_pointer -= (argc * 4) + 12;
        ((uint32_t *)process_stack_pointer)[argc] = 0;
        for (uintptr_t i = 0; i < argc; i++) {
            uint8_t *offset = &binary_buffer[i * 128];
            offset[127] = '\0';
            ((uint32_t *)process_stack_pointer)[i + 2] = (uintptr_t) offset;
            kprintf("pushing 0x%X\n", offset);
        }
        ((uint32_t *)process_stack_pointer)[1] = (uintptr_t) process_stack_pointer + 8;
        ((uint32_t *)process_stack_pointer)[0] = argc;
    }

    // set the final stack pointer
    process_stack_pointer -= sizeof(process_context_t);
    process->context = (process_context_t *) process_stack_pointer;

    // parse the APP binary and set EIP
    process->context->eip = relocate_app(binary_buffer + PROCESS_CODE_OFFSET);
    if (!process->context->eip) {
        kprintf("failed to parse and prepare the APP file for new process: %s\n", path);
        free((void *) process->stack_ptr_to_free);
        free(process);
        close(&binary);
        free(binary_buffer);
        return 0;
    }

    process->pid = new_pid;
    process->state = RUNNABLE;
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
