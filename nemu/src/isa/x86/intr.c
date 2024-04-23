#include <cpu/exec.h>
#include "local-include/rtl.h"

void raise_intr(DecodeExecState* s, word_t NO, vaddr_t ret_addr) {
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * That is, use ``NO'' to index the IDT.
     */

    //TODO();
    printf("int execution, NO == %x\n", NO);
    uint32_t idt = cpu.idtr_base;
    printf("int execution, idt == %x\n", idt);
    uint32_t lo = vaddr_read(idt + 8 * NO, 2);
    uint32_t hi = vaddr_read(idt + 8 * NO + 6, 2);
    printf("int execution, addr == %x\n", (hi << 16) | lo);

    cpu.eflags.IF = false;

    rtl_push(s, &cpu.eflags.val);
    rtl_push(s, &cpu.cs);
    rtl_push(s, &ret_addr);

    rtl_j(s, (hi << 16) | lo);
}

void query_intr(DecodeExecState *s) {
  TODO();
}
