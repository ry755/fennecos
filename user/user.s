
.equ SYS_yield_process, 1
.equ SYS_draw_string,   2

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC yield_process
SYSC draw_string
