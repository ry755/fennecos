#pragma once

#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

int kprintf(const char* __restrict, ...);
int printf(const char* __restrict, ...);
int vprintf(const char* __restrict, va_list);
int putchar(int);
int puts(const char*);
