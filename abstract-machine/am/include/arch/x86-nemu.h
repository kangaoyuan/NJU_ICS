#ifndef ARCH_H__
#define ARCH_H__

#include <stdint.h>
// Context as a interrupt handler parameter type, the address to access from low to high is the reverse order of push.
// The stack push order is eflags, cs, eip, irp, eax, ecx, edx, ebx, esp, ebp, esi, edi, cr3.
struct Context {
    // uintptr_t esi, ebx, eax, eip, edx, eflags, ecx, cs, esp, edi, ebp;
    void*     cr3;
    uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    int       irq;
    uintptr_t eip, cs, eflags;
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
