#include <kernel/allocator.h>
#include <kernel/event.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct event_queue_s {
    size_t head;
    size_t tail;
    size_t size;
    event_t *data;
} event_queue_t;

event_t event_queue_buffer[sizeof(event_t) * 32];
event_queue_t event_queue = { 0, 0, 32, (event_t *) &event_queue_buffer };
event_t empty_event = { .type = 0xFFFFFFFF };

static event_t read_event_queue(event_queue_t *queue) {
    if (queue->tail == queue->head)
        return empty_event;
    event_t data = queue->data[queue->tail];
    queue->data[queue->tail] = empty_event;
    queue->tail = (queue->tail + 1) % queue->size;
    return data;
}

static bool write_event_queue(event_queue_t *queue, event_t data) {
    if (((queue->head + 1) % queue->size) == queue->tail)
        return false;
    queue->data[queue->head] = data;
    queue->head = (queue->head + 1) % queue->size;
    return true;
}

void new_event(event_t *event) {
    write_event_queue(&event_queue, *event);
}

bool get_next_event(event_t *event) {
    event_t next_event = read_event_queue(&event_queue);
    if (next_event.type != 0xFFFFFFFF) {
        memcpy(event, &next_event, sizeof(event_t));
        return true;
    } else {
        return false;
    }
}
