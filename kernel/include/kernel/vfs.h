#pragma once

#include <kernel/queue.h>

#include <fatfs/ff.h>

#include <stdbool.h>
#include <stdint.h>

#define MODE_READ   0x01
#define MODE_WRITE  0x02
#define MODE_CREATE 0x04

#define BUFFER_SIZE 128

typedef enum file_type_e {
    T_FILE   = 0,
    T_STREAM = 1,
    T_DIR    = 2
} file_type_t;

typedef enum file_system_e {
    S_UNKNOWN = 0,
    S_FAT     = 1
} file_system_t;

typedef struct directory_s {
    uint32_t size;
    char name[256];
    DIR fatfs;
} directory_t;

typedef struct file_s {
    file_type_t type;
    file_system_t filesystem;
    FIL fatfs;
    directory_t dir;
    queue_t stream_queue;
    uint8_t stream_queue_data[BUFFER_SIZE];
} file_t;

uint32_t get_unused_file_id();
bool open(file_t *file, char *path, uint8_t mode);
bool close(file_t *file);
uint32_t read(file_t *file, char *buffer, uint32_t bytes_to_read);
uint32_t write(file_t *file, char *buffer, uint32_t bytes_to_write);
bool unlink(char *path);
bool chdir(char *path);
char *strip_last_path_component(char *path);
