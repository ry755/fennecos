#include <kernel/process.h>
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

bool open(file_t *file, char *path, uint8_t mode) {
    char full_path[256];
    if (*path == ':') {
        // this is a stream
        // TODO: implment this
        file->type = T_STREAM;
        return false;
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
            // TODO: implement
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
            // TODO: implement
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
