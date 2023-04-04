#pragma once

#include <stdint.h>

void insl(int port, void *addr, int cnt);
uint8_t inb(uint16_t port);
uint8_t inw(uint16_t port);
uint8_t inl(uint16_t port);
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);
