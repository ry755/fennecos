#pragma once

#include <kernel/trapframe.h>

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY_BASE 0x03F0
#define FLOPPY_IRQ 6
#define FLOPPY_DMA_BASE 0x8000
#define FLOPPY_DMA_LENGTH 0x4800

enum floppy_registers_e {
    FLOPPY_DOR  = 2, // digital output register
    FLOPPY_MSR  = 4, // master status register, read only
    FLOPPY_FIFO = 5, // data FIFO, in DMA operation for commands
    FLOPPY_CCR  = 7  // configuration control register, write only
};

enum floppy_commands_e {
    CMD_SPECIFY = 3,
    CMD_WRITE_DATA = 5,
    CMD_READ_DATA = 6,
    CMD_RECALIBRATE = 7,
    CMD_SENSE_INTERRUPT = 8,
    CMD_SEEK = 15
};

enum floppy_motor_e {
    floppy_motor_off  = 0,
    floppy_motor_on   = 1,
    floppy_motor_wait = 2
};

typedef enum floppy_dir_e {
    floppy_dir_read  = 1,
    floppy_dir_write = 2
} floppy_dir_t;

void init_floppy();
void floppy_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error);
void floppy_motor(uint32_t base, int onoff);
void floppy_motor_kill(int base);
void floppy_timer() ;
void floppy_write_cmd(uint32_t base, uint8_t cmd);
uint8_t floppy_read_data(uint32_t base);
void floppy_check_interrupt(uint32_t base, uint32_t *st0, uint32_t *cyl);
int32_t floppy_calibrate(uint32_t base);
uint32_t floppy_reset(uint32_t base);
int32_t floppy_seek(uint32_t base, uint32_t cyli, uint32_t head);
int32_t floppy_do_track(uint32_t base, uint32_t cyl, floppy_dir_t dir);
int32_t floppy_read_track(uint32_t base, uint32_t cyl);
int32_t floppy_write_track(uint32_t base, uint32_t cyl);
bool floppy_read_sector(uint32_t sector, uint8_t *buffer);
void lba_to_chs(uint32_t lba, uint16_t *cyl, uint16_t *head, uint16_t *sector);
