SECTIONS {
    . = 0x08000000;

    .text : {
        *(.text*)
    }

    .rodata ALIGN(16) : {
        *(.rodata*)
    }

    .data ALIGN(16) : {
        *(.data)
        *(.bss)
        *(COMMON)
    }
}
