#include <kernel/process.h>
#include <kernel/queue.h>
#include <kernel/vfs.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern process_t *current_process;

static file_system_t get_filesystem(char *path) {
    switch (*path) {
        case '0':
            return S_FAT;

        case '1':
            return S_FAT;

        default:
            return S_UNKNOWN;
    }
}

static bool open_stream(file_t *file, char *path) {
    // TODO: implement this
    (void) file;
    (void) path;
    return false;
}

uint32_t get_unused_file_id() {
    for (uint32_t i = 0; i < 16; i++)
        if (!current_process->files[i])
            return i;
    return (uint32_t) -1;
}

bool open(file_t *file, char *path, uint8_t mode) {
    char full_path[256];
    if (*path == ':') {
        // this is a stream
        file->type = T_STREAM;
        return open_stream(file, path);
    }
    if (*(path + 1) != ':') {
        // this is not an absolute path
        // we need to append it to the process's current directory
        strcpy(full_path, current_process->current_directory);
        strcat(full_path, "/");
        strcat(full_path, path);
        path = full_path;
    }

    file->filesystem = get_filesystem(path);

    switch (file->filesystem) {
        case S_FAT: {
            // in this case, the VFS `mode` and FatFS `mode` are compatible
            // so no translation is needed
            FRESULT result = f_open(&file->fatfs, path, mode);
            if (result != FR_OK)
                return false;
            file->type = T_FILE;
            return true;
        }

        default:
        case S_UNKNOWN:
            kprintf("vfs: attempted to open file on unknown filesystem: %s\n", path);
            return false;
    }
}

bool close(file_t *file) {
    switch (file->filesystem) {
        case S_FAT: {
            FRESULT result = f_close(&file->fatfs);
            if (result != FR_OK)
                return false;
            return true;
        }

        default:
        case S_UNKNOWN:
            kprintf("vfs: attempted to close file on unknown filesystem\n");
            return false;
    }
}

uint32_t read(file_t *file, char *buffer, uint32_t bytes_to_read) {
    switch (file->type) {
        case T_FILE: {
            switch (file->filesystem) {
                case S_FAT: {
                    unsigned int bytes_read;
                    f_read(&file->fatfs, buffer, bytes_to_read, &bytes_read);
                    return bytes_read;
                }

                default:
                case S_UNKNOWN:
                    kprintf("vfs: attempted to read file on unknown filesystem\n");
                    return 0;
            }
        }

        case T_STREAM: {
            for (uint32_t i = 0; i < bytes_to_read; i++)
                *(buffer + i) = read_queue(&file->stream_queue);
            return bytes_to_read;
        }

        default:
            return 0;
    }
}

uint32_t write(file_t *file, char *buffer, uint32_t bytes_to_write) {
    switch (file->type) {
        case T_FILE: {
            switch (file->filesystem) {
                case S_FAT: {
                    unsigned int bytes_written;
                    f_write(&file->fatfs, buffer, bytes_to_write, &bytes_written);
                    return bytes_written;
                }

                default:
                case S_UNKNOWN:
                    kprintf("vfs: attempted to write file on unknown filesystem\n");
                    return 0;
            }
        }

        case T_STREAM: {
            uint32_t bytes_written = 0;
            for (uint32_t i = 0; i < bytes_to_write; i++)
                if (write_queue(&file->stream_queue, *(buffer + i)))
                    bytes_written++;
            return bytes_written;
        }

        default:
            return 0;
    }
}

char *strip_last_path_component(char *path) {
    char *original_path = path;
    path += strlen(path) - 1;
    if (*path == '/')
        path--;
    if (*path == ':') {
        path++;
        return path;
    }
    while (*path-- != '/');
    *++path = 0;
    return original_path;
}
