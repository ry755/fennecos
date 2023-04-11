.global _start
_start:
    call main
    mov $0x1, %eax
    int $48
1:
    hlt
    jmp 1b
