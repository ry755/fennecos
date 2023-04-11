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
#include <kernel/process.h>
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

multiboot_info_t copied_multiboot_struct;

extern page_directory_t *kernel_page_directory;

void kernel_main(multiboot_info_t *multiboot_struct) {
    init_gdt();
    init_serial();
    init_pit();
    init_pic();
    init_ide();
    init_timer();
    init_ps2_keyboard();
    init_idt();
    memcpy(&copied_multiboot_struct, multiboot_struct, sizeof(multiboot_info_t));
    init_paging();
    init_allocator();
    init_scheduler();
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
    uint8_t font[8 * 16 * 256];
    FIL font_file;
    result = f_open(&font_file, "1:/res/font.bin", FA_READ);
    if (result != FR_OK) {
        kprintf("failed to open 1:/res/font.bin\n");
        kprintf("error: %d\n", result);
        abort();
    }
    unsigned int font_bytes_read;
    f_read(&font_file, &font, 8 * 16 * 256, &font_bytes_read);
    if (font_bytes_read != 8 * 16 * 256)
        kprintf("font file read short\n");
    f_close(&font_file);

    // initialize the framebuffer
    init_framebuffer(copied_multiboot_struct.framebuffer_addr, copied_multiboot_struct.framebuffer_pitch, copied_multiboot_struct.framebuffer_bpp, 0xFF1E1E2E, font, 8, 16);

    // run a test binary
    new_process("1:/bin/test.elf", NULL);

    // enter the scheduler
    kprintf("entering scheduler\n");
    scheduler();
    kprintf("scheduler returned, all processes died?\n");

    // unmount the hard disk and die if the scheduler returns
    kprintf("unmounting hard disk\n");
    result = f_unmount("1:");
    if (result != FR_OK) {
        kprintf("failed to unmount 1:\n");
        kprintf("error: %d\n", result);
        abort();
    }

    abort();
}
