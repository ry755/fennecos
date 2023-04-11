#pragma once

#include <sys/cdefs.h>

__attribute__((__noreturn__))
void abort(void);
char* itoa(int, char*, int);
char* utoa(unsigned int, char*, int);
