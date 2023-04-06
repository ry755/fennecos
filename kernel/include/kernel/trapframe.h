#pragma once

#include <stdint.h>

typedef struct trap_frame_s {
    // registers as pushed by pusha
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // rest of the trap frame
    uint16_t gs;
    uint16_t padding1;
    uint16_t fs;
    uint16_t padding2;
    uint16_t es;
    uint16_t padding3;
    uint16_t ds;
    uint16_t padding4;
    uint32_t trapno;

    // defined by x86 hardware
    uint32_t err;
    uint32_t eip;
    uint16_t cs;
    uint16_t padding5;
    uint32_t eflags;
} __attribute__((packed)) trap_frame_t;
