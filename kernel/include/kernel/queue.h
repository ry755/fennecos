#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct queue_s {
    size_t head;
    size_t tail;
    size_t size;
    uint8_t *data;
} queue_t;

uint8_t read_queue(queue_t *queue);
bool write_queue(queue_t *queue, uint8_t data);
