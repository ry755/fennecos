// implemented based on https://forum.osdev.org/viewtopic.php?t=13538

/******************************************************************************
* Copyright (c) 2007 Teemu Voipio                                             *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL    *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
******************************************************************************/

#include <kernel/floppy.h>
#include <kernel/io.h>
#include <kernel/isr.h>
#include <kernel/process.h>
#include <kernel/trapframe.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

volatile bool floppy_interrupt_occurred = false;
uint8_t *floppy_dma_buffer = (void *) FLOPPY_DMA_BASE;
static volatile uint32_t floppy_motor_ticks = 0;
static volatile uint32_t floppy_motor_state = 0;
static uint32_t cyl_in_buffer = 0xFFFFFFFF;

extern page_directory_t *current_page_directory;
extern page_directory_t *kernel_page_directory;

static inline void wait_for_floppy_interrupt() {
    while (!floppy_interrupt_occurred);
    floppy_interrupt_occurred = false;
}

static void floppy_dma_init(floppy_dir_t dir) {
    union {
        unsigned char b[4];
        unsigned long l;
    } a, c; // address and count

    a.l = (unsigned) floppy_dma_buffer;
    c.l = (unsigned) FLOPPY_DMA_LENGTH  - 1; // -1 because of DMA counting

    // check that address is at most 24-bits (under 16MB)
    // check that count is at most 16-bits (DMA limit)
    // check that if we add count and address we don't get a carry
    // (DMA can't deal with such a carry, this is the 64k boundary limit)
    if((a.l >> 24) || (c.l >> 16) || (((a.l & 0xFFFF) + c.l) >> 16)) {
        kprintf("floppy_dma_init: static buffer problem\n");
        return;
    }

    unsigned char mode;
    switch(dir) {
        // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
        case floppy_dir_read:
            mode = 0x46;
            break;

        // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
        case floppy_dir_write:
            mode = 0x4A;
            break;

        default:
            kprintf("floppy_dma_init: invalid direction");
            return;
    }

    outb(0x0A, 0x06);   // mask chan 2

    outb(0x0C, 0xFF);   // reset flip-flop
    outb(0x04, a.b[0]); //  - address low byte
    outb(0x04, a.b[1]); //  - address high byte

    outb(0x81, a.b[2]); // external page register

    outb(0x0C, 0xFF);   // reset flip-flop
    outb(0x05, c.b[0]); //  - count low byte
    outb(0x05, c.b[1]); //  - count high byte

    outb(0x0B, mode);   // set mode (see above)

    outb(0x0A, 0x02);   // unmask chan 2
}

void init_floppy() {
    install_interrupt_handler(FLOPPY_IRQ, floppy_interrupt_handler);
    for (uint32_t i = 0; i < 5; i++)
        map_physical_to_virtual(kernel_page_directory, FLOPPY_DMA_BASE + (i * 0x1000), FLOPPY_DMA_BASE + (i * 0x1000), true, true);
    floppy_reset(FLOPPY_BASE);
}

void floppy_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;
    floppy_interrupt_occurred = true;
}

void floppy_motor(uint32_t base, int onoff) {
    if (onoff) {
        if (!floppy_motor_state) {
            // need to turn on
            outb(base + FLOPPY_DOR, 0x1C);
            sleep_process(50); // wait 500 ms = hopefully enough for modern drives
        }
        floppy_motor_state = floppy_motor_on;
    } else {
        if (floppy_motor_state == floppy_motor_wait) {
            kprintf("floppy_motor: strange, fd motor-state already waiting\n");
        }
        //floppy_motor_ticks = 300; // 3 seconds, see floppy_timer() below
        //floppy_motor_state = floppy_motor_wait;
        sleep_process(50);
        floppy_motor_kill(FLOPPY_BASE);
    }
}

void floppy_motor_kill(int base) {
    outb(base + FLOPPY_DOR, 0x0C);
    floppy_motor_state = floppy_motor_off;
}

/*void floppy_timer() {
    while (true) {
        sleep_process(50);
        if (floppy_motor_state == floppy_motor_wait) {
            floppy_motor_ticks -= 50;
            if (floppy_motor_ticks <= 0) {
                floppy_motor_kill(FLOPPY_BASE);
            }
        }
    }
}*/

void floppy_write_cmd(uint32_t base, uint8_t cmd) {
    for (uint16_t i = 0; i < 100; i++) {
        sleep_process(1); // sleep 10 ms
        if (inb(base + FLOPPY_MSR) & 0x80) {
            outb(base + FLOPPY_FIFO, cmd);
            return;
        }
    }
    kprintf("floppy_write_cmd: timeout\n");
}

uint8_t floppy_read_data(uint32_t base) {
    for (uint16_t i = 0; i < 100; i++) {
        sleep_process(1); // sleep 10 ms
        if (inb(base + FLOPPY_MSR) & 0x80) {
            return inb(base + FLOPPY_FIFO);
        }
    }
    kprintf("floppy_read_data timeout\n");
    return 0;
}

void floppy_check_interrupt(uint32_t base, uint32_t *st0, uint32_t *cyl) {
    floppy_write_cmd(base, CMD_SENSE_INTERRUPT);

    *st0 = floppy_read_data(base);
    *cyl = floppy_read_data(base);
}

int32_t floppy_calibrate(uint32_t base) {
    uint32_t st0, cyl = -1; // set to bogus cylinder

    floppy_motor(base, floppy_motor_on);

    for (uint32_t i = 0; i < 10; i++) {
        // attempt to positions head to cylinder 0
        floppy_write_cmd(base, CMD_RECALIBRATE);
        floppy_write_cmd(base, 0); // argument is drive, we only support 0

        wait_for_floppy_interrupt();
        floppy_check_interrupt(base, &st0, &cyl);

        if (st0 & 0xC0) {
            static const char * status[] = { 0, "error", "invalid", "drive" };
            kprintf("floppy_calibrate: status = %s\n", status[st0 >> 6]);
            continue;
        }

        if (!cyl) { // found cylinder 0 ?
            floppy_motor(base, floppy_motor_off);
            return 0;
        }
    }

    kprintf("floppy_calibrate: 10 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;
}

uint32_t floppy_reset(uint32_t base) {
    outb(base + FLOPPY_DOR, 0x00); // disable controller
    outb(base + FLOPPY_DOR, 0x0C); // enable controller

    wait_for_floppy_interrupt();

    for (uint8_t i = 0; i < 4; i++) {
        uint32_t st0, cyl;
        floppy_check_interrupt(base, &st0, &cyl);
    }

    // set transfer speed 500kb/s
    outb(base + FLOPPY_CCR, 0x00);

    // configure for implied seek on, FIFO on, drive polling mode off, threshold = 8, precompensation 0
    floppy_write_cmd(base, CMD_CONFIGURE);
    floppy_write_cmd(base, 0x00);
    floppy_write_cmd(base, 0x57);
    floppy_write_cmd(base, 0x00);

    //  - 1st byte is: bits[7:4] = steprate, bits[3:0] = head unload time
    //  - 2nd byte is: bits[7:1] = head load time, bit[0] = no-DMA
    //
    //  steprate    = (8.0ms - entry*0.5ms)*(1MB/s / xfer_rate)
    //  head_unload = 8ms * entry * (1MB/s / xfer_rate), where entry 0 -> 16
    //  head_load   = 1ms * entry * (1MB/s / xfer_rate), where entry 0 -> 128
    //
    floppy_write_cmd(base, CMD_SPECIFY);
    floppy_write_cmd(base, 0xDF); /* steprate = 3ms, unload time = 240ms */
    floppy_write_cmd(base, 0x02); /* load time = 16ms, no-DMA = 0 */

    // it could fail
    if (floppy_calibrate(base)) return -1;
    else return 0;
}

int32_t floppy_do_track(uint32_t base, uint32_t cyl, floppy_dir_t dir) {
    // transfer command, set below
    uint8_t cmd;

    // Read is MT:MF:SK:0:0:1:1:0, write MT:MF:0:0:1:0:1
    // where MT = multitrack, MF = MFM mode, SK = skip deleted
    //
    // Specify multitrack and MFM mode
    static const int flags = 0xC0;
    switch (dir) {
        case floppy_dir_read:
            cmd = CMD_READ_DATA | flags;
            break;

        case floppy_dir_write:
            cmd = CMD_WRITE_DATA | flags;
            break;

        default:
            kprintf("floppy_do_track: invalid direction\n");
            return 0;
    }

    for (uint32_t i = 0; i < 10; i++) {
        floppy_motor(base, floppy_motor_on);

        floppy_dma_init(dir);

        sleep_process(5); // give some time (50ms) to settle

        floppy_write_cmd(base, cmd);  // set above for current direction
        floppy_write_cmd(base, 0);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        floppy_write_cmd(base, cyl);  // cylinder
        floppy_write_cmd(base, 0);    // first head (should match with above)
        floppy_write_cmd(base, 1);    // first sector, strangely counts from 1
        floppy_write_cmd(base, 2);    // bytes/sector, 128*2^x (x=2 -> 512)
        floppy_write_cmd(base, 18);   // number of tracks to operate on
        floppy_write_cmd(base, 0x1B); // GAP3 length, 27 is default for 3.5"
        floppy_write_cmd(base, 0xFF); // data length (0xff if B/S != 0)

        wait_for_floppy_interrupt();

        // first read status information
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = floppy_read_data(base);
        st1 = floppy_read_data(base);
        st2 = floppy_read_data(base);
        rcy = floppy_read_data(base);
        rhe = floppy_read_data(base);
        rse = floppy_read_data(base);
        // bytes per sector, should be what we programmed in
        bps = floppy_read_data(base);
        (void) rcy;
        (void) rhe;
        (void) rse;

        int error = 0;

        if (st0 & 0xC0) {
            static const char * status[] = { 0, "error", "invalid command", "drive not ready" };
            kprintf("floppy_do_track: status = %s\n", status[st0 >> 6]);
            error = 1;
        }
        if (st1 & 0x80) {
            kprintf("floppy_do_track: end of cylinder\n");
            error = 1;
        }
        if (st0 & 0x08) {
            kprintf("floppy_do_track: drive not ready\n");
            error = 1;
        }
        if (st1 & 0x20) {
            kprintf("floppy_do_track: CRC error\n");
            error = 1;
        }
        if (st1 & 0x10) {
            kprintf("floppy_do_track: controller timeout\n");
            error = 1;
        }
        if (st1 & 0x04) {
            kprintf("floppy_do_track: no data found\n");
            error = 1;
        }
        if ((st1 | st2) & 0x01) {
            kprintf("floppy_do_track: no address mark found\n");
            error = 1;
        }
        if (st2 & 0x40) {
            kprintf("floppy_do_track: deleted address mark\n");
            error = 1;
        }
        if (st2 & 0x20) {
            kprintf("floppy_do_track: CRC error in data\n");
            error = 1;
        }
        if (st2 & 0x10) {
            kprintf("floppy_do_track: wrong cylinder\n");
            error = 1;
        }
        if (st2 & 0x04) {
            kprintf("floppy_do_track: uPD765 sector not found\n");
            error = 1;
        }
        if (st2 & 0x02) {
            kprintf("floppy_do_track: bad cylinder\n");
            error = 1;
        }
        if (bps != 0x2) {
            kprintf("floppy_do_track: wanted 512B/sector, got %d\n", (1<<(bps+7)));
            error = 1;
        }
        if (st1 & 0x02) {
            kprintf("floppy_do_track: not writable\n");
            error = 2;
        }

        if (!error) {
            floppy_motor(base, floppy_motor_off);
            return 0;
        }
        if (error > 1) {
            kprintf("floppy_do_track: not retrying\n");
            floppy_motor(base, floppy_motor_off);
            return -2;
        }
    }

    kprintf("floppy_do_track: 10 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;
}

int32_t floppy_read_track(uint32_t base, uint32_t cyl) {
    return floppy_do_track(base, cyl, floppy_dir_read);
}

int32_t floppy_write_track(uint32_t base, uint32_t cyl) {
    return floppy_do_track(base, cyl, floppy_dir_write);
}

bool floppy_read_sector(uint32_t sector, uint8_t *buffer) {
    uint16_t cyl_read;
    uint16_t head_read;
    uint16_t sector_read;
    int32_t read_result = 0;

    lba_to_chs(sector, &cyl_read, &head_read, &sector_read);

    // if write support is ever added, this will need a flag to
    // determine when the buffer is in a dirty state
    if (cyl_read != cyl_in_buffer)
        read_result = floppy_read_track(FLOPPY_BASE, cyl_read);

    if (!read_result) {
        memcpy(buffer, &floppy_dma_buffer[(sector % (18 * 2)) * 512], 512);
        cyl_in_buffer = cyl_read;
        return true;
    } else {
        return false;
    }
}

void lba_to_chs(uint32_t lba, uint16_t *cyl, uint16_t *head, uint16_t *sector) {
    if (cyl) *cyl = lba / (2 * 18);
    if (head) *head = ((lba % (2 * 18)) / 18);
    if (sector) *sector = ((lba % (2 * 18)) % 18 + 1);
}
