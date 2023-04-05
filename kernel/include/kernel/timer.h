#pragma once

#include <stdint.h>

void init_timer();
void timer_interrupt_handler();
uint32_t get_timer_value();
