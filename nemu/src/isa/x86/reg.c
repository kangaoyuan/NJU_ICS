#include <isa.h>
#include "local-include/reg.h"
#include <time.h>
#include <stdlib.h>

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
    srand(time(0));

    word_t sample[8];
    for (int i = R_EAX; i <= R_EDI; i++) {
        sample[i] = rand();
        reg_l(i) = sample[i];
        assert(reg_w(i) == (sample[i] & 0xffff));
    }

    assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
    assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
    assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
    assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
    assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
    assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
    assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
    assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

    assert(sample[R_EAX] == cpu.eax);
    assert(sample[R_ECX] == cpu.ecx);
    assert(sample[R_EDX] == cpu.edx);
    assert(sample[R_EBX] == cpu.ebx);
    assert(sample[R_ESP] == cpu.esp);
    assert(sample[R_EBP] == cpu.ebp);
    assert(sample[R_ESI] == cpu.esi);
    assert(sample[R_EDI] == cpu.edi);

    word_t pc_sample = rand();
    cpu.pc = pc_sample;
    assert(pc_sample == cpu.pc);
}

void isa_reg_display() {
    printf("$eip\t0x%08x\n", cpu.pc);
    for (int i = R_EAX; i <= R_EDI; i++) {
        printf("$%s\t0x%08x\n", regsl[i], reg_l(i));
    }
}

word_t isa_reg_str2val(const char *s, bool *success) {
    *success = true;
    if (strcmp(s, "pc") == 0 || strcmp(s, "eip") == 0)
        return cpu.pc;
    for (int i = R_EAX; i <= R_EDI; i++) {
        if (strcmp(s, regsl[i]) == 0)
            return reg_l(i);
    }
    *success = false;
    return 0;
}
