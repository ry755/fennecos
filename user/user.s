.equ SYS_exit_process,  1
.equ SYS_yield_process, 2
.equ SYS_draw_string,   3

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC exit_process
SYSC yield_process
SYSC draw_string
