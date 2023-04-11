.equ SYS_exit,           1
.equ SYS_yield,          2
.equ SYS_new_process,    3
.equ SYS_open,           4
.equ SYS_close,          5
.equ SYS_read,           6
.equ SYS_write,          7
.equ SYS_new_event,      8
.equ SYS_get_next_event, 9

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC exit
SYSC yield
SYSC new_process
SYSC open
SYSC close
SYSC read
SYSC write
SYSC new_event
SYSC get_next_event
