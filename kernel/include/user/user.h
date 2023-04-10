#pragma once

#include <user/framebuffer.h>

#include <stdint.h>

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

font_t *global_font = (void *) 0xFF000000;

uint32_t exit();
uint32_t yield();
uint32_t new_event(event_t *event);
uint32_t get_next_event(event_t *event);
