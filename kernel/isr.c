#include <kernel/isr.h>
#include <kernel/pic.h>

#include <stdint.h>
#include <stdio.h>

void exception_handler(uint8_t num, uint32_t error) {
    printf("INTERRUPT/EXCEPTION: num = %d, error = 0x%x\n", num, error);
    end_of_interrupt(num);
}
