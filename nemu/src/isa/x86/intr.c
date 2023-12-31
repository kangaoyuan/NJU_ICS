#include <cpu/exec.h>
#include "local-include/rtl.h"

void raise_intr(DecodeExecState* s, word_t NO, vaddr_t ret_addr) {
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * That is, use ``NO'' to index the IDT. */

    //printf("Firstly, int $%x intruction exec helper\n", NO);
    cpu.eflags.IF = 0;  // Set for disable intr
    rtl_push(s, &cpu.eflags.val);
    rtl_push(s, &cpu.cs);
    rtl_push(s, &ret_addr);  // rtl_push(&cpu.pc);

    Assert(NO < cpu.idtr_limit, "Bigger than IDTR limit.");
    //get GateDesc32 gate; init_irq() -> cte_init() -> set_idt() to do it.
    vaddr_t offset = cpu.idtr_base + 8 * NO;
    word_t gate_low = vaddr_read(offset, 2);
    word_t gate_high = vaddr_read(offset + 6, 2);

    // gate.offset_31_16 << 16 | gate.offset_15_0 
    word_t int_addr = gate_high << 16 | gate_low;
    rtl_j(s, int_addr);
}

void query_intr(DecodeExecState* s) {
    /*if (cpu.INTR == 1 && cpu.eflags.IF == 1) {
        cpu.INTR = false;
        raise_intr(s, 32, cpu.pc);
        update_pc(s);
    }*/
}
