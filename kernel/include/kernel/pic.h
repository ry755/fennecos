#pragma once

#include <stdint.h>

void init_pic();
void pic_unmask(uint8_t irq);
void end_of_interrupt(uint8_t irq);
