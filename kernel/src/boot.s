.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp

    /* push multiboot header struct pointer */
    push %ebx

    call kernel_main

    cli
1:  hlt
    jmp 1b

.global flush_gdt
.extern gdt_ptr
flush_gdt:
    lgdt [gdt_ptr]
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    jmp $0x08,$flush
flush:
    ret
