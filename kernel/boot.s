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

    /* TODO: set up GDT and paging */

    /* push multiboot header struct pointer */
    push %ebx

    call kernel_main

    cli
1:	hlt
    jmp 1b

.size _start, . - _start
