#include <kernel/allocator.h>
#include <kernel/elf.h>
#include <kernel/framebuffer.h>
#include <kernel/gdt.h>
#include <kernel/ide.h>
#include <kernel/idt.h>
#include <kernel/io.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/pic.h>
#include <kernel/pit.h>
#include <kernel/ps2.h>
#include <kernel/ramdisk.h>
#include <kernel/serial.h>
#include <kernel/timer.h>

#include <fatfs/ff.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t font[FONT_WIDTH * FONT_HEIGHT * 256];

void kernel_main(multiboot_info_t *multiboot_struct) {
    init_gdt();
    init_serial();
    init_pit();
    init_pic();
    init_ide();
    init_timer();
    init_ps2_keyboard();
    init_idt();
    init_paging();
    init_allocator();
    init_framebuffer(multiboot_struct->framebuffer_addr, 0xFF123456);
    //init_ramdisk();

    // mount the hard disk
    kprintf("mounting hard disk\n");
    FATFS fs;
    FRESULT result = f_mount(&fs, "1:", 1);
    if (result != FR_OK) {
        kprintf("failed to mount 1:\n");
        kprintf("error: %d\n", result);
        abort();
    }

    // open and read the font file
    FIL font_file;
    result = f_open(&font_file, "1:/res/font.bin", FA_READ);
    if (result != FR_OK) {
        kprintf("failed to open 1:/res/font.bin\n");
        kprintf("error: %d\n", result);
        abort();
    }
    unsigned int font_bytes_read;
    f_read(&font_file, &font, FONT_WIDTH * FONT_HEIGHT * 256, &font_bytes_read);
    if (font_bytes_read != FONT_WIDTH * FONT_HEIGHT * 256)
        kprintf("font file read short\n");
    f_close(&font_file);

    draw_string("hello world!", 16, 16, 0xFFFFFFFF, 0xFF123456);

    FIL file_to_write;
    result = f_open(&file_to_write, "1:/hello.txt", FA_WRITE | FA_CREATE_NEW);
    if (result != FR_OK) {
        kprintf("failed to open 1:/hello.txt\n");
        kprintf("error: %d\n", result);
        abort();
    }

    unsigned int bytes_written;
    f_write(&file_to_write, "hello world!!\nif you're seeing this then fatfs is working properly :3", 69, &bytes_written);
    if (bytes_written != 69)
        kprintf("log file written short\n");
    f_close(&file_to_write);

    page_directory_t *elf_page_directory = (page_directory_t *) kallocate(sizeof(page_directory_t), true, NULL);
    memset(elf_page_directory, 0, sizeof(page_directory_t));
    for (uint32_t i = 1; i < 0x01000000; i += 0x1000)
        map_physical_to_virtual(elf_page_directory, i, i, true, true);
    uint32_t v = 0xF0000000;
    for (uint32_t i = multiboot_struct->framebuffer_addr; i < multiboot_struct->framebuffer_addr + 640*480*4; i += 0x1000) {
        map_physical_to_virtual(elf_page_directory, i, v, true, true);
        v += 0x1000;
    }
    FIL elf_file;
    uint8_t elf_file_buffer[700] __attribute__ ((aligned (4096)));
    map_physical_to_virtual(elf_page_directory, (uint32_t) &elf_file_buffer[0], 0x08048000, true, true);
    switch_page_directory(elf_page_directory);
    result = f_open(&elf_file, "1:/test.elf", FA_READ);
    if (result != FR_OK) {
        kprintf("failed to open 1:/test.elf\n");
        kprintf("error: %d\n", result);
        abort();
    }
    unsigned int elf_bytes_read;
    f_read(&elf_file, &elf_file_buffer, 700, &elf_bytes_read);
    if (elf_bytes_read != 700)
        kprintf("binary file read short\n");
    f_close(&elf_file);
    execute_elf(&elf_file_buffer);

    // unmount the hard disk
    kprintf("unmounting hard disk\n");
    result = f_unmount("1:");
    if (result != FR_OK) {
        kprintf("failed to unmount 1:\n");
        kprintf("error: %d\n", result);
        abort();
    }

    abort();
}
