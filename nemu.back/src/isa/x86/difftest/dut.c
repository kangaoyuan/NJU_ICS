#include <isa.h>
#include <monitor/difftest.h>
#include "../local-include/reg.h"
#include "difftest.h"

bool isa_difftest_checkregs(CPU_state* ref_r, vaddr_t pc) {
    bool flag[10];
    flag[0] = difftest_check_reg("eax", pc, ref_r->eax, cpu.eax);
    flag[1] = difftest_check_reg("ecx", pc, ref_r->ecx, cpu.ecx);
    flag[2] = difftest_check_reg("edx", pc, ref_r->edx, cpu.edx);
    flag[3] = difftest_check_reg("ebx", pc, ref_r->ebx, cpu.ebx);
    flag[4] = difftest_check_reg("esp", pc, ref_r->esp, cpu.esp);
    flag[5] = difftest_check_reg("ebp", pc, ref_r->ebp, cpu.ebp);
    flag[6] = difftest_check_reg("esi", pc, ref_r->esi, cpu.esi);
    flag[7] = difftest_check_reg("edi", pc, ref_r->edi, cpu.edi);
    flag[8] = difftest_check_reg("pc", pc, ref_r->pc, cpu.pc);
    flag[9] = difftest_check_reg("eflags", pc, ref_r->eflags.val, cpu.eflags.val);
    for (int i = 0; i < 10; i++) {
        if (!flag[i])
            return false;
    }
    return true;
}

void isa_difftest_attach() {
}
