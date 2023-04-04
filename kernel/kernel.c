#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/framebuffer.h>
#include <kernel/ide.h>
#include <kernel/io.h>
#include <kernel/multiboot.h>
#include <kernel/serial.h>

#include <fatfs/ff.h>

extern uint8_t font[FONT_WIDTH * FONT_HEIGHT * 256];

void kernel_main(multiboot_info_t *multiboot_struct) {
    init_serial();
    init_framebuffer(multiboot_struct->framebuffer_addr, 0xFF123456);

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

    draw_string("hello world!", 16, 16, 0xFFFFFFFF, 0xFF123456);

    abort();
}
