#include <cpu/exec.h>
#include "local-include/rtl.h"

void raise_intr(DecodeExecState* s, word_t NO, vaddr_t ret_addr) {
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * That is, use ``NO'' to index the IDT. */

    cpu.eflags.IF = 0;  // Set for disable intr
    rtl_push(s, &cpu.eflags.val);
    rtl_push(s, &cpu.cs);
    rtl_push(s, &ret_addr);  // rtl_push(&cpu.eip);

    printf("cpu.idtr_limit%u\n", cpu.idtr_limit);
    vaddr_t offset = cpu.idtr_base + 8 * NO;
    Assert(NO < cpu.idtr_limit, "Bigger than IDTR limit.");
    //GateDesc32 gate;
    word_t gate_low = vaddr_read(offset, 2);
    word_t gate_high = vaddr_read(offset + 6, 2);

    // gate.offset_31_16 << 16 | gate.offset_15_0 
    rtl_j(s, gate_high << 16 | gate_low);
}

void query_intr(DecodeExecState* s) {
    TODO();
}
