#include <kernel/allocator.h>
#include <kernel/elf.h>
#include <kernel/floppy.h>
#include <kernel/framebuffer.h>
#include <kernel/gdt.h>
#include <kernel/ide.h>
#include <kernel/idt.h>
#include <kernel/io.h>
#include <kernel/mouse.h>
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

#define HD_FONT_PATH "1:/res/font.bin"
#define FD_FONT_PATH "2:/res/font.bin"
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define HD_INIT_PATH "1:/app/console.app"
#define FD_INIT_PATH "2:/app/console.app"

multiboot_info_t copied_multiboot_struct;
uint8_t temporary_font[FONT_WIDTH * FONT_HEIGHT * 256];

extern page_directory_t *kernel_page_directory;

void kernel_main(multiboot_info_t *multiboot_struct) {
    init_gdt();
    init_serial();
    init_pit();
    init_pic();
    init_ide();
    init_timer();
    init_ps2_keyboard();
    init_mouse();
    init_idt();
    memcpy(&copied_multiboot_struct, multiboot_struct, sizeof(multiboot_info_t));
    init_paging();
    init_allocator();
    init_scheduler();
    init_floppy();

    bool booting_from_floppy = (copied_multiboot_struct.boot_device >> 24) == 0x00 ? true : false;

    // mount the hard disk
    kprintf("mounting hard disk\n");
    FATFS hard_disk;
    FRESULT result = f_mount(&hard_disk, "1:", 1);
    if (result != FR_OK) {
        kprintf("failed to mount 1:\n");
        kprintf("error: %d\n", result);
    }

    // initialize the ramdisk
    FATFS ram_disk;
    init_ramdisk(&ram_disk, "1:/res/ramdisk.img");

    // mount the floppy disk
    kprintf("mounting floppy disk\n");
    FATFS floppy_disk;
    result = f_mount(&floppy_disk, "2:", 1);
    if (result != FR_OK) {
        kprintf("failed to mount 2:\n");
        kprintf("error: %d\n", result);
    }

    // open and read the font file
    FIL font_file;
    char *font_bin = (booting_from_floppy ? FD_FONT_PATH : HD_FONT_PATH);
    result = f_open(&font_file, font_bin, FA_READ);
    if (result != FR_OK) {
        kprintf("failed to open font.bin from boot disk\n");
        kprintf("error: %d\n", result);
        abort();
    }
    unsigned int font_bytes_read;
    f_read(&font_file, temporary_font, FONT_WIDTH * FONT_HEIGHT * 256, &font_bytes_read);
    if (font_bytes_read != FONT_WIDTH * FONT_HEIGHT * 256)
        kprintf("font file read short\n");
    f_close(&font_file);

    // initialize the framebuffer
    init_framebuffer(
        copied_multiboot_struct.framebuffer_addr,
        copied_multiboot_struct.framebuffer_pitch,
        copied_multiboot_struct.framebuffer_bpp,
        0x1E1E2E, temporary_font, FONT_WIDTH, FONT_HEIGHT
    );

    // create stdin and stdout and run the console
    file_t stdin_file = {
        .type = T_STREAM,
        .stream_queue.head = 0,
        .stream_queue.tail = 0,
        .stream_queue.size = BUFFER_SIZE,
        .stream_queue.data = stdin_file.stream_queue_data
    };
    file_t stdout_file = {
        .type = T_STREAM,
        .stream_queue.head = 0,
        .stream_queue.tail = 0,
        .stream_queue.size = BUFFER_SIZE,
        .stream_queue.data = stdout_file.stream_queue_data
    };

    char *init = (booting_from_floppy ? FD_INIT_PATH : HD_INIT_PATH);
    new_process(init, NULL, &stdin_file, &stdout_file);

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
