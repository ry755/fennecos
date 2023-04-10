#!/bin/bash

set -e
TOOLCHAIN_PATH=~/opt/cross/bin/

kernel_input_files=(
    "kernel/kernel.c"
    "kernel/allocator.c"
    "kernel/elf.c"
    "kernel/event.c"
    "kernel/framebuffer.c"
    "kernel/gdt.c"
    "kernel/ide.c"
    "kernel/idt.c"
    "kernel/io.c"
    "kernel/isr.c"
    "kernel/paging.c"
    "kernel/process.c"
    "kernel/pic.c"
    "kernel/pit.c"
    "kernel/ps2.c"
    "kernel/queue.c"
    "kernel/ramdisk.c"
    "kernel/serial.c"
    "kernel/syscall.c"
    "kernel/timer.c"

    "kernel/fatfs/diskio.c"
    "kernel/fatfs/ff.c"
    "kernel/fatfs/ffsystem.c"
    "kernel/fatfs/ffunicode.c"

    "kernel/syscall/sys_exit.c"
    "kernel/syscall/sys_yield.c"
    "kernel/syscall/sys_new_event.c"
    "kernel/syscall/sys_get_next_event.c"

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
    "libc/string/strcpy.c"
    "libc/string/strcat.c"
)

kernel_output_files=(
    "build/kernel/kernel.o"
    "build/kernel/allocator.o"
    "build/kernel/elf.o"
    "build/kernel/event.o"
    "build/kernel/framebuffer.o"
    "build/kernel/gdt.o"
    "build/kernel/ide.o"
    "build/kernel/idt.o"
    "build/kernel/io.o"
    "build/kernel/isr.o"
    "build/kernel/paging.o"
    "build/kernel/process.o"
    "build/kernel/pic.o"
    "build/kernel/pit.o"
    "build/kernel/ps2.o"
    "build/kernel/queue.o"
    "build/kernel/ramdisk.o"
    "build/kernel/serial.o"
    "build/kernel/syscall.o"
    "build/kernel/timer.o"

    "build/kernel/fatfs/diskio.o"
    "build/kernel/fatfs/ff.o"
    "build/kernel/fatfs/ffsystem.o"
    "build/kernel/fatfs/ffunicode.o"

    "build/kernel/syscall/sys_exit.o"
    "build/kernel/syscall/sys_yield.o"
    "build/kernel/syscall/sys_new_event.o"
    "build/kernel/syscall/sys_get_next_event.o"

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
    "build/libc/string/strcpy.o"
    "build/libc/string/strcat.o"
)

mkdir -p build/kernel/{fatfs,syscall}
mkdir -p build/libc/{stdio,stdlib,string}
mkdir -p build/user/bin

mkdir -p base_image/bin

# kernel
${TOOLCHAIN_PATH}i686-elf-as kernel/boot.s -o build/kernel/boot.o
for file in "${kernel_input_files[@]}"; do
    ${TOOLCHAIN_PATH}i686-elf-gcc -c "$file" -o "build/${file%.*}.o" -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel/include/ -Ilibc/include/
done
${TOOLCHAIN_PATH}i686-elf-gcc -T kernel/linker.ld -o base_image/boot/fennecos.elf -ffreestanding -O2 -nostdlib build/kernel/boot.o "${kernel_output_files[@]}" -lgcc
nm base_image/boot/fennecos.elf -p | grep ' T \| t ' | awk '{ print $1" "$3 }' > base_image/boot/fennecos.sym

# user
# this needs some major cleanup
${TOOLCHAIN_PATH}i686-elf-gcc -c user/user.s -o build/user/user.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
${TOOLCHAIN_PATH}i686-elf-gcc -c user/framebuffer.c -o build/user/framebuffer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel/include/ -Ilibc/include/
${TOOLCHAIN_PATH}i686-elf-gcc -c user/keyboard.c -o build/user/keyboard.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel/include/ -Ilibc/include/
${TOOLCHAIN_PATH}i686-elf-gcc -c user/bin/test.c -o build/user/bin/test.o -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel/include/ -Ilibc/include/
${TOOLCHAIN_PATH}i686-elf-gcc -o base_image/bin/test.elf -ffreestanding -O2 -nostdlib build/user/user.o build/user/framebuffer.o build/user/keyboard.o build/user/bin/test.o -lgcc

sudo bash image.sh
