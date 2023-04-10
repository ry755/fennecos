.equ SYS_exit_process,   1
.equ SYS_yield_process,  2
.equ SYS_new_event,      3
.equ SYS_get_next_event, 4

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC exit_process
SYSC yield_process
SYSC new_event
SYSC get_next_event
