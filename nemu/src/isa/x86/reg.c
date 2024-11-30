#include <isa.h>
#include <time.h>
#include <stdlib.h>
#include "local-include/reg.h"

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
    word_t sample[8];
    srand(time(NULL));

    for (int i = R_EAX; i <= R_EDI; i++) {
        sample[i] = rand();
        reg_l(i) = sample[i];
        // reg_w 16 bits.
        assert(reg_w(i) == (sample[i] & 0xffff));
    }

    // reg_b 8 bits.
    assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
    assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
    assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
    assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
    assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
    assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
    assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
    assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

    // cpu.eax - cpu.edi 32 bits.
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

    // cpu.pc 32 bits
    assert(pc_sample == cpu.pc);
}

// This function is called by "(nemu) info r" cmd.
void isa_reg_display() {
    printf("$eip\t0x%08x\n", cpu.pc);
    for (int i = R_EAX; i <= R_EDI; i++) {
        printf("$%s\t0x%08x\t%08u\n", regsl[i], reg_l(i), reg_l(i));
    }
    printf("SF=%d\n", cpu.eflags.SF&1);
	printf("ZF=%d\n", cpu.eflags.ZF&1);
	printf("OF=%d\n", cpu.eflags.OF&1);
    printf("CF=%d\n", cpu.eflags.CF&1);
}

word_t isa_reg_str2val(const char* s, bool* success) {
    if (!strcmp("pc", s) || !strcmp("eip", s)) {
        *success = true;
        return cpu.pc;
    }
    for (int i = R_EAX; i <= R_EDI; i++) {
        if (!strcmp(regsl[i], s)) {
            *success = true;
            return reg_l(i);
        }
    }
    for (int i = R_AX; i <= R_DI; i++) {
        if (!strcmp(regsw[i], s)) {
            *success = true;
            return reg_w(i);
        }
    }
    for (int i = R_AL; i <= R_BH; i++) {
        if (!strcmp(regsb[i], s)) {
            *success = true;
            return reg_b(i);
        }
    }
    *success = false;
    panic("isa_reg_str2val failed");
    return -1;
}
