.equ SYS_exit,           1
.equ SYS_yield,          2
.equ SYS_new_event,      3
.equ SYS_get_next_event, 4

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC exit
SYSC yield
SYSC new_event
SYSC get_next_event
