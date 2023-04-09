#include <kernel/event.h>
#include <kernel/syscall.h>

#include <stdint.h>

uint32_t sys_new_event() {
    new_event((event_t *) fetch_syscall_u32(0));
    return 0;
}
