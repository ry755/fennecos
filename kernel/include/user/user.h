#pragma once

#include <fatfs/ff.h>

#include <user/framebuffer.h>

#include <stdint.h>

#define MODE_READ   0x01
#define MODE_WRITE  0x02
#define MODE_CREATE 0x04

typedef enum event_type_e {
    KEY_DOWN,
    KEY_UP
} event_type_t;

typedef struct event_s {
    event_type_t type;
    uint32_t arg0;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
    uint32_t arg5;
    uint32_t arg6;
    uint32_t arg7;
} event_t;

typedef struct directory_s {
    uint32_t size;
    char name[256];
} directory_t;

uint32_t exit();
uint32_t yield();
uint32_t new_process(char path[], char *argv[]);
uint32_t open(char *path, uint32_t mode);
uint32_t close(uint32_t file_id);
uint32_t read(uint32_t file_id, char *buffer, uint32_t bytes_to_read);
uint32_t write(uint32_t file_id, char *buffer, uint32_t bytes_to_write);
char *cwd();
uint32_t chdir(char *dir);
uint32_t new_event(event_t *event);
uint32_t get_next_event(event_t *event);
