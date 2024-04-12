#include <isa.h>
#include <stdlib.h>
#include <time.h>
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

void isa_reg_display() {
}

word_t isa_reg_str2val(const char* s, bool* success) {
    if (!strcmp("$pc", s)) {
        *success = true;
        return cpu.pc;
    }
    for (int i = R_EAX; i <= R_EDI; i++) {
        if (!strcmp(regsl[i], s + 1)) {
            *success = true;
            return reg_l(i);
        }
    }
    *success = false;
    panic("isa_reg_str2val failed");
    return -1;
}
