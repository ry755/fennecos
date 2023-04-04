#include <kernel/io.h>

#include <stdint.h>

inline void insl(int port, void *addr, int cnt) {
    asm volatile("cld; rep insl" :
                "=D" (addr), "=c" (cnt) :
                "d" (port), "0" (addr), "1" (cnt) :
                "memory", "cc"
    );
}

inline uint8_t inb(uint16_t port) {
    uint8_t return_value;
    asm volatile ( "inb %1, %0" : "=a"(return_value) : "Nd"(port) );
    return return_value;
}

inline uint8_t inw(uint16_t port) {
    uint16_t return_value;
    asm volatile ( "inw %1, %0" : "=a"(return_value) : "Nd"(port) );
    return return_value;
}

inline uint8_t inl(uint16_t port) {
    uint32_t return_value;
    asm volatile ( "inl %1, %0" : "=a"(return_value) : "Nd"(port) );
    return return_value;
}

inline void outsl(int port, const void *addr, int cnt) {
    asm volatile("cld; rep outsl" :
                "=S" (addr), "=c" (cnt) :
                "d" (port), "0" (addr), "1" (cnt) :
                "cc"
    );
}

inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

inline void outw(uint16_t port, uint16_t val) {
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

inline void outl(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}
