/* https://www.gnu.org/software/grub/manual/multiboot/multiboot.html */

/* Declare constants for the multiboot header. */
.set ALIGN,     1<<0                        /* align loaded modules on page boundaries */
.set MEMINFO,   1<<1                        /* provide memory map */
.set VIDEOMODE, 1<<2                        /* set video mode */
.set FLAGS,     ALIGN | MEMINFO | VIDEOMODE /* this is the Multiboot 'flag' field */
.set MAGIC,     0x1BADB002                  /* 'magic number' lets bootloader find the header */
.set CHECKSUM,  -(MAGIC + FLAGS)            /* checksum of above, to prove we are multiboot */

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* padding */
.long 0x00000000
.long 0x00000000
.long 0x00000000
.long 0x00000000
.long 0x00000000

/* for VIDEOMODE */
.long 0x00000000 /* mode_type */
.long 640        /* width */
.long 480        /* height */
.long 32         /* depth */

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

.extern interrupt_handler
.macro ISR_NOERR num
.global isr_stub_\num
isr_stub_\num:
    pushl $0x00000000
    push $\num
    call interrupt_handler
    cli
1:  hlt
    jmp 1b
.endm
.macro ISR_ERR num
.global isr_stub_\num
isr_stub_\num:
    push $\num
    call interrupt_handler
    cli
1:  hlt
    jmp 1b
.endm
.macro IRQ_ENTRY num, irq
.global irq_stub_\num
irq_stub_\num:
    pusha
    pushl $0x00000000
    push $\irq
    call interrupt_handler
    add $8, %esp
    popa
    iret
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

IRQ_ENTRY 0, 32
IRQ_ENTRY 1, 33
IRQ_ENTRY 2, 34
IRQ_ENTRY 3, 35
IRQ_ENTRY 4, 36
IRQ_ENTRY 5, 37
IRQ_ENTRY 6, 38
IRQ_ENTRY 7, 39
IRQ_ENTRY 8, 40
IRQ_ENTRY 9, 41
IRQ_ENTRY 10, 42
IRQ_ENTRY 11, 43
IRQ_ENTRY 12, 44
IRQ_ENTRY 13, 45
IRQ_ENTRY 14, 46
IRQ_ENTRY 15, 47

.global isr_stub_table
isr_stub_table:
    .long isr_stub_0
    .long isr_stub_1
    .long isr_stub_2
    .long isr_stub_3
    .long isr_stub_4
    .long isr_stub_5
    .long isr_stub_6
    .long isr_stub_7
    .long isr_stub_8
    .long isr_stub_9
    .long isr_stub_10
    .long isr_stub_11
    .long isr_stub_12
    .long isr_stub_13
    .long isr_stub_14
    .long isr_stub_15
    .long isr_stub_16
    .long isr_stub_17
    .long isr_stub_18
    .long isr_stub_19
    .long isr_stub_20
    .long isr_stub_21
    .long isr_stub_22
    .long isr_stub_23
    .long isr_stub_24
    .long isr_stub_25
    .long isr_stub_26
    .long isr_stub_27
    .long isr_stub_28
    .long isr_stub_29
    .long isr_stub_30
    .long isr_stub_31
    .long irq_stub_0
    .long irq_stub_1
    .long irq_stub_2
    .long irq_stub_3
    .long irq_stub_4
    .long irq_stub_5
    .long irq_stub_6
    .long irq_stub_7
    .long irq_stub_8
    .long irq_stub_9
    .long irq_stub_10
    .long irq_stub_11
    .long irq_stub_12
    .long irq_stub_13
    .long irq_stub_14
    .long irq_stub_15

.size _start, . - _start
