#include <am.h>

Area heap;
#define nemu_trap(code) asm volatile (".byte 0xd6" : :"a"(code))

void putch(char ch) {
    putchar(ch);
}

void halt(int code) {
    nemu_trap(code);

    while(true)
        ;
}
