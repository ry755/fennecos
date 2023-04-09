#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct queue_s {
    size_t head;
    size_t tail;
    size_t size;
    void **data;
} queue_t;

void *read_queue(queue_t *queue);
bool write_queue(queue_t *queue, void *handle);
