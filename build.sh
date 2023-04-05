#!/bin/bash

set -e
TOOLCHAIN_PATH=~/opt/cross/bin/

input_files=(
    "kernel/kernel.c"
    "kernel/framebuffer.c"
    "kernel/gdt.c"
    "kernel/ide.c"
    "kernel/idt.c"
    "kernel/io.c"
    "kernel/isr.c"
    "kernel/pic.c"
    "kernel/pit.c"
    "kernel/ramdisk.c"
    "kernel/serial.c"

    "kernel/fatfs/diskio.c"
    "kernel/fatfs/ff.c"
    "kernel/fatfs/ffsystem.c"
    "kernel/fatfs/ffunicode.c"

    "libc/stdio/kprintf.c"
    "libc/stdio/printf.c"
    "libc/stdio/putchar.c"
    "libc/stdio/puts.c"
    "libc/stdio/vprintf.c"

    "libc/stdlib/abort.c"
    "libc/stdlib/itoa.c"
    "libc/stdlib/utoa.c"

    "libc/string/memcmp.c"
    "libc/string/memcpy.c"
    "libc/string/memmove.c"
    "libc/string/memset.c"
    "libc/string/strlen.c"
    "libc/string/strchr.c"
)

output_files=(
    "build/kernel/kernel.o"
    "build/kernel/framebuffer.o"
    "build/kernel/gdt.o"
    "build/kernel/ide.o"
    "build/kernel/idt.o"
    "build/kernel/io.o"
    "build/kernel/isr.o"
    "build/kernel/pic.o"
    "build/kernel/pit.o"
    "build/kernel/ramdisk.o"
    "build/kernel/serial.o"

    "build/kernel/fatfs/diskio.o"
    "build/kernel/fatfs/ff.o"
    "build/kernel/fatfs/ffsystem.o"
    "build/kernel/fatfs/ffunicode.o"

    "build/libc/stdio/kprintf.o"
    "build/libc/stdio/printf.o"
    "build/libc/stdio/putchar.o"
    "build/libc/stdio/puts.o"
    "build/libc/stdio/vprintf.o"

    "build/libc/stdlib/abort.o"
    "build/libc/stdlib/itoa.o"
    "build/libc/stdlib/utoa.o"

    "build/libc/string/memcmp.o"
    "build/libc/string/memcpy.o"
    "build/libc/string/memmove.o"
    "build/libc/string/memset.o"
    "build/libc/string/strlen.o"
    "build/libc/string/strchr.o"
)

mkdir -p build/kernel/fatfs
mkdir -p build/libc/{stdio,stdlib,string}

${TOOLCHAIN_PATH}i686-elf-as kernel/boot.s -o build/kernel/boot.o

for file in "${input_files[@]}"; do
    ${TOOLCHAIN_PATH}i686-elf-gcc -c "$file" -o "build/${file%.*}.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel/include/ -Ilibc/include/
done

${TOOLCHAIN_PATH}i686-elf-gcc -T kernel/linker.ld -o base_image/boot/fennecos.elf -ffreestanding -O2 -nostdlib build/kernel/boot.o "${output_files[@]}" -lgcc
nm base_image/boot/fennecos.elf -p | grep ' T \| t ' | awk '{ print $1" "$3 }' > base_image/boot/fennecos.sym

sudo bash image.sh
