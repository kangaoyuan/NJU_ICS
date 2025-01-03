#include "cc.h"
#include "../local-include/decode.h" // IWYU pragma: keep

// remeber
/*
 *t0, t1, ... - Intermediate results can only be stored during the implementation of the rtl-pseudo instruction
 *s0, s1, ... - Intermediate results can only be stored during the implementation of decode and execute auxiliary functions.
 */

static inline def_EHelper(jmp) {
    // the target address is calculated at the decode stage
    rtl_j(s, s->jmp_pc);
    print_asm("jmp %x", s->jmp_pc);
}

static inline def_EHelper(jcc) {
    // the target address is calculated at the decode stage
    uint32_t cc = s->opcode & 0xf;
    switch(id_dest->width){
    case 1:
        s->jmp_pc = s->seq_pc + (int8_t)(*ddest);
        break;
    case 2:
        s->jmp_pc = s->seq_pc + (int16_t)(*ddest);
        break;
    case 4: 
        s->jmp_pc = s->seq_pc + (int32_t)(*ddest);
        break;
    }
    *s0 = 0;
    rtl_setcc(s, s0, cc);
    rtl_jrelop(s, RELOP_NE, s0, rz, s->jmp_pc);
    print_asm("j%s %x", get_cc_name(cc), s->jmp_pc);
}

static inline def_EHelper(jmp_rm) {
    rtl_jr(s, ddest);
    print_asm("jmp *%s", id_dest->str);
}

static inline def_EHelper(call) {
    // the target address is calculated at the decode stage
    // TODO();
    rtl_push(s, &s->seq_pc);
    rtl_j(s, s->jmp_pc);
    print_asm("call %x", s->jmp_pc);
}

static inline def_EHelper(ret) {
    // TODO();
    rtl_pop(s, &s->jmp_pc);
    rtl_j(s, s->jmp_pc);
    print_asm("ret");
}

static inline def_EHelper(ret_imm) {
    print_asm("ret %s", id_dest->str);
}

static inline def_EHelper(call_rm) {
    // TODO();
    rtl_push(s, &s->seq_pc);
    rtl_jr(s, ddest);
    print_asm("call *%s", id_dest->str);
}
