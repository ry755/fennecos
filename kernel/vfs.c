#include <kernel/process.h>
#include <kernel/queue.h>
#include <kernel/vfs.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern process_t *current_process;

char boot_disk_char = '0';

static file_system_t get_filesystem(char *path) {
    char first = path[0];
    if (first == '/') first = boot_disk_char;
    switch (first) {
        case '0':
            return S_FAT;

        case '1':
            return S_FAT;

        case '2':
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

static bool open_file(file_t *file, char *path, uint8_t mode) {
    // in this case, the VFS `mode` and FatFS `mode` are compatible
    // so no translation is needed
    FRESULT result = f_open(&file->fatfs, path, mode);
    if (result != FR_OK)
        return false;
    file->type = T_FILE;
    return true;
}

static bool open_dir(file_t *file, char *path) {
    FRESULT result = f_opendir(&file->dir.fatfs, path);
    if (result != FR_OK)
        return false;
    file->type = T_DIR;
    return true;
}

uint32_t get_unused_file_id() {
    for (uint32_t i = 0; i < 16; i++)
        if (!current_process->files[i])
            return i;
    return (uint32_t) -1;
}

bool open(file_t *file, char *path, uint8_t mode) {
    if (*path == ':') {
        // this is a stream
        file->type = T_STREAM;
        return open_stream(file, path);
    }

    char full_path[256];
    path = make_absolute_path(path, full_path);

    file->filesystem = get_filesystem(path);

    switch (file->filesystem) {
        case S_FAT: {
            if (!open_file(file, path, mode)) {
                if (!open_dir(file, path)) {
                    kprintf("vfs: failed to open: %s\n", path);
                    return false;
                }
            }
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
            switch (file->type) {
                case T_FILE: {
                    FRESULT result = f_close(&file->fatfs);
                    if (result != FR_OK)
                        return false;
                    return true;
                }

                case T_DIR: {
                    FRESULT result = f_closedir(&file->dir.fatfs);
                    if (result != FR_OK)
                        return false;
                    return true;
                }

                default:
                    return false;
            }
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

        case T_DIR: {
            switch (file->filesystem) {
                case S_FAT: {
                    FILINFO info;
                    directory_t dir_info;
                    f_readdir(&file->dir.fatfs, &info);
                    dir_info.size = info.fsize;
                    strcpy(dir_info.name, info.fname);
                    if (bytes_to_read >= sizeof(directory_t)) {
                        memcpy(buffer, &dir_info, sizeof(directory_t));
                        return sizeof(directory_t);
                    } else {
                        return 0;
                    }
                }

                default:
                case S_UNKNOWN:
                    kprintf("vfs: attempted to read dir on unknown filesystem\n");
                    return 0;
            }
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

        case T_DIR: {
            kprintf("vfs: attempted to write to directory\n");
            return 0;
        }

        default:
            return 0;
    }
}

bool seek(file_t *file, uint32_t offset) {
    switch (file->type) {
        case T_FILE: {
            switch (file->filesystem) {
                case S_FAT: {
                    return f_lseek(&file->fatfs, offset) == FR_OK;
                }

                default:
                case S_UNKNOWN:
                    kprintf("vfs: attempted to seek file on unknown filesystem\n");
                    return false;
            }
        }

        case T_STREAM: {
            return false;
        }

        case T_DIR: {
            kprintf("vfs: attempted to seek directory\n");
            return false;
        }

        default:
            return false;
    }
}

// FIXME: f_unlink() always errors?
bool unlink(char *path) {
    if (*path == ':') {
        // this is a stream
        return false;
    }

    char full_path[256];
    path = make_absolute_path(path, full_path);

    file_system_t filesystem = get_filesystem(path);

    switch (filesystem) {
        case S_FAT: {
            if (f_unlink(path) == FR_OK)
                return true;
            else
                return false;
        }

        default:
        case S_UNKNOWN:
            kprintf("vfs: attempted to unlink file on unknown filesystem: %s\n", path);
            return false;
    }
}

bool chdir(char *path) {
    file_t dir;
    char full_path[256];

    // FIXME: this sucks
    if (!strcmp(path, "..")) {
        strcpy(full_path, current_process->current_directory);
        path = strip_last_path_component(full_path);
    } else {
        path = make_absolute_path(path, full_path);
    }

    if (!open_dir(&dir, path)) return false;
    dir.filesystem = get_filesystem(path);

    strcpy(current_process->current_directory, path);
    uint32_t length = strlen(current_process->current_directory);
    if (current_process->current_directory[length - 1] != '/') {
        current_process->current_directory[length] = '/';
        current_process->current_directory[length + 1] = '\0';
    }

    close(&dir);
    return true;
}

char *strip_last_path_component(char *path) {
    char *original_path = path;
    path += strlen(path) - 1;
    if (*path == '/')
        path--;
    if (path == original_path)
        return original_path;
    while (*path-- != '/');
    path++;
    *++path = 0;
    return original_path;
}

char *make_absolute_path(char *path, char *full_path) {
    if (path[0] == '/') {
        // this is an absolute path that refers to the boot disk
        full_path[0] = boot_disk_char;
        full_path[1] = ':';
        full_path[2] = 0;
        strcat(full_path, path);
        return full_path;
    }
    if (path[1] != ':') {
        // this is not an absolute path
        // we need to append it to the process's current directory
        strcpy(full_path, current_process->current_directory);
        strcat(full_path, path);

        // recurse to ensure "sh.app" -> "/app/sh.app" -> "2:/app/sh.app"
        char full_path_2[256];
        char *path_2 = full_path;
        path_2 = make_absolute_path(path_2, full_path_2);
        if (path_2 == full_path_2) {
            strcpy(full_path, path_2);
        }

        return full_path;
    }
    return path;
}
