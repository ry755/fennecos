// https://gist.github.com/ryankurte/61f95dc71133561ed055ff62b33585f8

#include <kernel/queue.h>

#include <stdbool.h>
#include <stddef.h>

void *read_queue(queue_t *queue) {
    if (queue->tail == queue->head)
        return NULL;
    void *handle = queue->data[queue->tail];
    queue->data[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->size;
    return handle;
}

bool write_queue(queue_t *queue, void *handle) {
    if (((queue->head + 1) % queue->size) == queue->tail)
        return false;
    queue->data[queue->head] = handle;
    queue->head = (queue->head + 1) % queue->size;
    return true;
}
