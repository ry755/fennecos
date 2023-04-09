#include <kernel/allocator.h>
#include <kernel/event.h>
#include <kernel/queue.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void *event_queue_buffer[sizeof(void *) * 32];
queue_t event_queue = { 0, 0, 32, (void **) &event_queue_buffer };

void new_event(event_t *event) {
    event_t *new_event = (event_t *) kallocate(sizeof(event_t), false, NULL);
    memcpy(new_event, event, sizeof(event_t));
    if (!write_queue(&event_queue, new_event)) free((void *) new_event);
}

bool get_next_event(event_t *event) {
    void *handle = read_queue(&event_queue);
    if (handle) {
        memcpy(event, handle, sizeof(event_t));
        free(handle);
        return true;
    } else {
        return false;
    }
}
