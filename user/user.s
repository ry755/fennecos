.equ SYS_draw_string, 1

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC draw_string
