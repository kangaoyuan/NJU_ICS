#ifndef ARCH_H__
#define ARCH_H__

struct Context {
    // TODO: fix the order of these members to match trap.S
    void*     cr3;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t       irq;
    uint32_t eip, cs, eflags;
};

#define GPR1 eax
#define GPR2 eip
#define GPR3 eip
#define GPR4 eip
#define GPRx eip

#endif
