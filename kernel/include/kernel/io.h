#pragma once

#include <stdint.h>

#define bochs_breakpoint() outw(0x8A00, 0x8A00); outw(0x8A00, 0x08AE0);

void insl(int port, void *addr, int cnt);
uint8_t inb(uint16_t port);
uint8_t inw(uint16_t port);
uint8_t inl(uint16_t port);
void outsl(int port, const void *addr, int cnt);
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);
