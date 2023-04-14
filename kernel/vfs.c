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
            if (!open_file(file, path, mode)) {
                if (!open_dir(file, path)) {
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

bool unlink(char *path) {
    char full_path[256];
    if (*path == ':') {
        // this is a stream
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

// TODO: use the rest of the VFS once it is sufficiently capable
bool chdir(char *path) {
    DIR dir;
    char new_path[256];

    // FIXME: this sucks
    if (!strcmp(path, "..")) {
        strcpy(new_path, current_process->current_directory);
        path = strip_last_path_component(new_path);
    }

    // FIXME: this sucks even more
    if (f_opendir(&dir, path) != FR_OK) {
        f_closedir(&dir);
        strcpy(new_path, current_process->current_directory);
        if (new_path[strlen(new_path) - 1] != '/')
            strcat(new_path, "/");
        strcat(new_path, path);
        path = new_path;
        if (f_opendir(&dir, path) != FR_OK)
            return false;
    }

    strcpy(current_process->current_directory, path);

    f_closedir(&dir);
    return true;
}

char *strip_last_path_component(char *path) {
    char *original_path = path;
    path += strlen(path) - 1;
    if (*path == '/')
        path--;
    if (*path == ':')
        return original_path;
    while (*path-- != '/');
    path++;
    *++path = 0;
    return original_path;
}
