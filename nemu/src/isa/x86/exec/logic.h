#include "cc.h"
#include "../local-include/decode.h"

static inline def_EHelper(setcc) {
    uint32_t cc = s->opcode & 0xf;
    rtl_setcc(s, s0, cc);
    operand_write(s, id_dest, s0);
    print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

static inline def_EHelper(test) {
    rtl_and(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    print_asm_template2(test);
}

static inline def_EHelper(and) {
    rtl_and(s, s0, ddest, dsrc1);
    operand_write(s, id_dest, s0);
    rtl_update_ZFSF(s, s0, id_dest->width);
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    print_asm_template2(and);
}

static inline def_EHelper(or) {
    rtl_or(s, s0, ddest, dsrc1);
    operand_write(s, id_dest, s0);
    rtl_update_ZFSF(s, s0, id_dest->width);
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    print_asm_template2(or);
}

static inline def_EHelper(xor) {
    rtl_xor(s, s0, ddest, dsrc1);
    operand_write(s, id_dest, s0);
    rtl_update_ZFSF(s, s0, id_dest->width);
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    print_asm_template2(xor);
}

static inline def_EHelper(not) {
    rtl_li(s, s0, BITMASK(8 * id_dest->width));
    rtl_xor(s, s1, ddest, s0);
    operand_write(s, id_dest, s1);
    rtl_update_ZFSF(s, s1, id_dest->width);
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    print_asm_template1(not);
}

static inline def_EHelper(sar) {
    rtl_sar(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    // unnecessary to update CF and OF in NEMU
    print_asm_template2(sar);
}

static inline def_EHelper(shl) {
    rtl_shl(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    // unnecessary to update CF and OF in NEMU
    print_asm_template2(shl);
}

static inline def_EHelper(shr) {
    rtl_shr(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    // unnecessary to update CF and OF in NEMU
    print_asm_template2(shr);
}

static inline def_EHelper(rol) {
    while(*dsrc1){
        rtl_msb(s, s0, ddest, id_dest->width);
        *ddest = *ddest * 2 + *s0;
        *dsrc1 -= 1;
    }
    rtl_update_ZFSF(s, ddest, id_dest->width);
    operand_write(s, id_dest, ddest);
    // unnecessary to update CF and OF in NEMU
    print_asm_template2(rol);
}

static inline def_EHelper(ror) {
    while(*dsrc1){
        rtl_andi(s, s0, ddest, 1);
        *ddest = *ddest / 2 + (*s0 << 31);
        *dsrc1 -= 1;
    }
    rtl_update_ZFSF(s, ddest, id_dest->width);
    operand_write(s, id_dest, ddest);
    // unnecessary to update CF and OF in NEMU
    print_asm_template2(ror);
}

static inline def_EHelper(shld) {
    *s0 = s->isa.is_operand_size_16 ? *dsrc1 % 16 : *dsrc1 % 32;
    *s1 = *dsrc2;
    if (*s0 != 0) {
        *t0 = ((*ddest >> (id_dest->width * 8 - *s0)) & 1);
        rtl_set_CF(s, t0);
        if (s->isa.is_operand_size_16) {
            rtl_shl(s, ddest, ddest, s0);
            rtl_andi(s, ddest, ddest, 0xffff);
            rtl_andi(s, s1, s1, 0xffff);
            rtl_shri(s, s1, s1, 16 - *s0);
            rtl_or(s, s1, s1, ddest);
        } else {
            rtl_shl(s, ddest, ddest, s0);
            rtl_shri(s, s1, s1, 32 - *s0);
            rtl_or(s, s1, s1, ddest);
        }
        rtl_update_ZFSF(s, s1, id_dest->width);
        operand_write(s, id_dest, s1);
    }
}

static inline def_EHelper(shrd) {
    *s0 = s->isa.is_operand_size_16 ? *dsrc1 % 16 : *dsrc1 % 32;
    *s1 = *dsrc2;       
    if (*s0 != 0) {
        *t0 = ((*ddest >> (*s0 - 1)) & 1);
        rtl_set_CF(s, t0);
        if (s->isa.is_operand_size_16) {
            rtl_shr(s, ddest, ddest, s0);
            rtl_andi(s, ddest, ddest, 0xffff);
            rtl_andi(s, s1, s1, 0xffff);
            rtl_shli(s, s1, s1, 16 - *s0);
            rtl_or(s, s1, s1, ddest);
            operand_write(s, id_dest, s1);
        } else {
            rtl_shr(s, ddest, ddest, s0);
            rtl_shli(s, s1, s1, 32 - *s0);
            rtl_or(s, s1, s1, ddest);
            operand_write(s, id_dest, s1);
        }
    }
}

static inline def_EHelper(bsr) {
    if (*dsrc1 == 0) {
        cpu.eflags.ZF = 0;
    } else {
        cpu.eflags.ZF = 1;
        *s0 = (s->isa.is_operand_size_16 ? 16 : 32) - 1;
        while ((*dsrc1 & (1 << *s0)) == 0) {
            rtl_subi(s, s0, s0, 1);
            *ddest = *s0;
        }
    }
}
