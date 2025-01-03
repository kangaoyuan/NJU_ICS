#include <cpu/exec.h>
#include "local-include/rtl.h"

void raise_intr(DecodeExecState* s, word_t NO, vaddr_t ret_addr) {
    printf("cpu triggers int\n");
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * That is, use ``NO'' to index the IDT.
     */
    if(NO >= cpu.idtr_limit){
        assert(0); 
    }

    // To support the switch between kernel and user address spaces.
    if((cpu.cs & 0x3) == 0){ // np
        /* From kernel interrupt */
        // Here we need vaddr_read to access the IDT content.
        uint32_t idt = cpu.idtr_base;
        uint32_t lo = vaddr_read(idt + 8 * NO, 2);
        uint32_t hi = vaddr_read(idt + 8 * NO + 6, 2);

        rtl_push(s, &cpu.eflags.val);
        rtl_push(s, &cpu.cs);
        rtl_push(s, &ret_addr);

        cpu.eflags.IF = false;

        rtl_j(s, (hi << 16) | lo);
    } else if((cpu.cs & 0x3) == 3){
        /* From user interrupt */
        // cte.c is the core to init.
        // Here the purpose is to switch to kernel stack to do the work.
        uint32_t gdt_addr = cpu.gdtr_base + cpu.TR;
        uint32_t tss_addr = vaddr_read(gdt_addr+2, 2) + (vaddr_read(gdt_addr+4, 1) << 16)
                            + (vaddr_read(gdt_addr+7, 1) << 24);

        uint32_t ss = cpu.ss;
        uint32_t esp = cpu.esp;
        uint32_t ss0 = vaddr_read(tss_addr+8, 4);
        uint32_t esp0 = vaddr_read(tss_addr+4, 4); // ksp
        cpu.ss = ss0;
        if(esp0 != 0)
            cpu.esp = esp0;
        rtl_push(s, &ss); // c->ss3
        rtl_push(s, &esp); // c->sp(c->esp3)
         
        // Here we need vaddr_read to access the IDT content.
        uint32_t idt = cpu.idtr_base;
        uint32_t lo = vaddr_read(idt + 8 * NO, 2);
        uint32_t hi = vaddr_read(idt + 8 * NO + 6, 2);

        rtl_push(s, &cpu.eflags.val);
        rtl_push(s, &cpu.cs);
        rtl_push(s, &ret_addr);

        cpu.eflags.IF = false;

        rtl_j(s, (hi << 16) | lo);

        // ksp = 0;
        vaddr_write(tss_addr+4, 0, 4);
    }
}

#define IRQ_TIMER 32
void query_intr(DecodeExecState *s) {
    // Modified by timer handler
    if(cpu.INTR && cpu.eflags.IF) {
        cpu.INTR = false; 
        raise_intr(s, IRQ_TIMER, cpu.pc);
        update_pc(s);
    }
}
