#include <kernel/event.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_get_next_event() {
    return get_next_event((event_t *) fetch_syscall_u32(0));
}
