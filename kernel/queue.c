// based on https://gist.github.com/ryankurte/61f95dc71133561ed055ff62b33585f8 with changes

#include <kernel/queue.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint8_t read_queue(queue_t *queue) {
    if (queue->tail == queue->head)
        return 0;
    uint8_t data = queue->data[queue->tail];
    queue->data[queue->tail] = 0;
    queue->tail = (queue->tail + 1) % queue->size;
    return data;
}

bool write_queue(queue_t *queue, uint8_t data) {
    if (((queue->head + 1) % queue->size) == queue->tail)
        return false;
    queue->data[queue->head] = data;
    queue->head = (queue->head + 1) % queue->size;
    return true;
}
