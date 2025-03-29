.equ SYS_exit,           1
.equ SYS_kill,           2
.equ SYS_yield,          3
.equ SYS_sleep,          4
.equ SYS_new_process,    5
.equ SYS_get_pid,        6
.equ SYS_open,           7
.equ SYS_close,          8
.equ SYS_read,           9
.equ SYS_write,          10
.equ SYS_seek,           11
.equ SYS_unlink,         12
.equ SYS_cwd,            13
.equ SYS_chdir,          14
.equ SYS_new_event,      15
.equ SYS_get_next_event, 16
.equ SYS_get_mouse,      17

.macro SYSC name
.global \name
\name:
    movl $SYS_\name, %eax
    int $48
    ret
.endm

SYSC exit
SYSC kill
SYSC yield
SYSC sleep
SYSC new_process
SYSC get_pid
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
