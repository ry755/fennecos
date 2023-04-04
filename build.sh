#!/bin/bash

set -e
TOOLCHAIN_PATH=~/opt/cross/bin/

input_files=(
    "kernel/kernel.c"
    "kernel/include/kernel/framebuffer.c"
    "kernel/include/kernel/ide.c"
    "kernel/include/kernel/io.c"
    "kernel/include/kernel/serial.c"

    "kernel/include/fatfs/diskio.c"
    "kernel/include/fatfs/ff.c"
    "kernel/include/fatfs/ffsystem.c"
    "kernel/include/fatfs/ffunicode.c"

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
    "build/kernel/include/kernel/framebuffer.o"
    "build/kernel/include/kernel/ide.o"
    "build/kernel/include/kernel/io.o"
    "build/kernel/include/kernel/serial.o"

    "build/kernel/include/fatfs/diskio.o"
    "build/kernel/include/fatfs/ff.o"
    "build/kernel/include/fatfs/ffsystem.o"
    "build/kernel/include/fatfs/ffunicode.o"

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

mkdir -p build/kernel/include/{kernel,fatfs}
mkdir -p build/libc/{stdio,stdlib,string}

${TOOLCHAIN_PATH}i686-elf-as kernel/boot.s -o build/kernel/boot.o

for file in "${input_files[@]}"; do
    ${TOOLCHAIN_PATH}i686-elf-gcc -c "$file" -o "build/${file%.*}.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel/include/ -Ilibc/include/
done

${TOOLCHAIN_PATH}i686-elf-gcc -T kernel/linker.ld -o base_image/boot/fennecos.bin -ffreestanding -O2 -nostdlib build/kernel/boot.o "${output_files[@]}" -lgcc

sudo bash image.sh
