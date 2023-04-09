#pragma once

#include <stdbool.h>
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

void new_event(event_t *event);
bool get_next_event(event_t *event);
