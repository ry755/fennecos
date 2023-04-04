#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/framebuffer.h>
#include <kernel/gdt.h>
#include <kernel/ide.h>
#include <kernel/io.h>
#include <kernel/multiboot.h>
#include <kernel/ramdisk.h>
#include <kernel/serial.h>

#include <fatfs/ff.h>

extern uint8_t font[FONT_WIDTH * FONT_HEIGHT * 256];

void kernel_main(multiboot_info_t *multiboot_struct) {
    init_gdt();
    init_serial();
    init_framebuffer(multiboot_struct->framebuffer_addr, 0xFF123456);
    //init_ramdisk();

    // mount the hard disk
    write_serial_string("mounting hard disk\n");
    FATFS fs;
    FRESULT result = f_mount(&fs, "1:", 1);
    if (result != FR_OK) {
        write_serial_string("failed to mount 1:\n");
        printf("error: %d\n", result);
        abort();
    }

    // open and read the font file
    FIL font_file;
    result = f_open(&font_file, "1:/res/font.bin", FA_READ);
    if (result != FR_OK) {
        write_serial_string("failed to open 1:/res/font.bin\n");
        printf("error: %d\n", result);
        abort();
    }
    unsigned int font_bytes_read;
    f_read(&font_file, &font, FONT_WIDTH * FONT_HEIGHT * 256, &font_bytes_read);
    if (font_bytes_read != FONT_WIDTH * FONT_HEIGHT * 256)
        write_serial_string("font file read short\n");
    f_close(&font_file);

    draw_string("hello world!", 16, 16, 0xFFFFFFFF, 0xFF123456);

    FIL file_to_write;
    result = f_open(&file_to_write, "1:/hello.txt", FA_WRITE | FA_CREATE_NEW);
    if (result != FR_OK) {
        write_serial_string("failed to open 1:/hello.txt\n");
        printf("error: %d\n", result);
        abort();
    }

    unsigned int bytes_written;
    f_write(&file_to_write, "hello world!!\nif you're seeing this then fatfs is working properly :3", 69, &bytes_written);
    if (bytes_written != 69)
        write_serial_string("log file written short\n");
    f_close(&file_to_write);

    // unmount the hard disk
    write_serial_string("unmounting hard disk\n");
    result = f_unmount("1:");
    if (result != FR_OK) {
        write_serial_string("failed to unmount 1:\n");
        printf("error: %d\n", result);
        abort();
    }

    abort();
}
