.equ SYS_exit,           1
.equ SYS_yield,          2
.equ SYS_sleep,          3
.equ SYS_new_process,    4
.equ SYS_open,           5
.equ SYS_close,          6
.equ SYS_read,           7
.equ SYS_write,          8
.equ SYS_seek,           9
.equ SYS_unlink,         10
.equ SYS_cwd,            11
.equ SYS_chdir,          12
.equ SYS_new_event,      13
.equ SYS_get_next_event, 14
.equ SYS_get_mouse,      15

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC exit
SYSC yield
SYSC sleep
SYSC new_process
SYSC open
SYSC close
SYSC read
SYSC write
SYSC seek
SYSC unlink
SYSC cwd
SYSC chdir
SYSC new_event
SYSC get_next_event
SYSC get_mouse
