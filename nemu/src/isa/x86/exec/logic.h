#include "cc.h"
#include "../local-include/decode.h"

static inline def_EHelper(test) {
    // TODO();
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    rtl_and(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    print_asm_template2(test);
}

static inline def_EHelper(and) {
    // TODO();
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    rtl_and(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template2(and);
}

static inline def_EHelper(xor) {
    // TODO();
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    rtl_xor(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template2(xor);
}

static inline def_EHelper(or) {
    // TODO();
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    rtl_or(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template2(or);
}

static inline def_EHelper(not) {
    // TODO();
    rtl_set_CF(s, rz);
    rtl_set_OF(s, rz);
    rtl_not(s, s0, ddest);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template1(not );
}

static inline def_EHelper(sar) {
    // TODO();
    // unnecessary to update CF and OF in NEMU
    rtl_sar(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template2(sar);
}

static inline def_EHelper(shl) {
    // TODO();
    // unnecessary to update CF and OF in NEMU
    rtl_shl(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template2(shl);
}

static inline def_EHelper(shr) {
    // TODO();
    // unnecessary to update CF and OF in NEMU
    rtl_shr(s, s0, ddest, dsrc1);
    rtl_update_ZFSF(s, s0, id_dest->width);
    operand_write(s, id_dest, s0);
    print_asm_template2(shr);
}

static inline def_EHelper(setcc) {
    uint32_t cc = s->opcode & 0xf;
    rtl_setcc(s, ddest, cc);
    operand_write(s, id_dest, ddest);

    print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

static inline def_EHelper(bsr){
    if(*dsrc1 == 0){
        cpu.eflags.ZF = 1; 
    } else {
        *s1 = id_dest->width * 8 - 1;
        cpu.eflags.ZF = 0;

        while((*dsrc1 >> *s1) == 0){
            (*s1)--; 
        } 
        operand_write(s, id_dest, s1);
    }
    print_asm_template2(bsr);
}

static inline def_EHelper(rol){
    if (*dsrc1 > 8 * id_dest->width)
        assert(0);
    rtl_shri(s, s0, ddest, 8 * id_dest->width - *dsrc1);
    rtl_shli(s, s1, ddest, *dsrc1);
    rtl_or(s, s1, s1, s0);
    operand_write(s, id_dest, s1);
    rtl_andi(s, s0, s0, 1);
    rtl_set_CF(s, s0);
    print_asm_template2(rol);
}

static inline def_EHelper(ror){
    if (*dsrc1 > 8 * id_dest->width)
        assert(0);
    rtl_shli(s, s0, ddest, 8 * id_dest->width - *dsrc1);
    rtl_shri(s, s1, ddest, *dsrc1);
    rtl_or(s, s1, s1, s0);
    operand_write(s, id_dest, s1);
    rtl_andi(s, s0, s0, 1);
    rtl_set_CF(s, s0);
    print_asm_template2(rol);
}

static inline def_EHelper(shld){
    rtl_mv(s, s0, dsrc1);
    rtl_mv(s, s1, dsrc2);
    *s0 %= 32;
    rtl_shl(s, ddest, ddest, s0);
    rtl_shri(s, s1, s1, 32-*s0);
    rtl_or(s, ddest, ddest, s1);
    operand_write(s, id_dest, ddest);
    print_asm_template2(shld);
}

static inline def_EHelper(shrd){
    rtl_mv(s, s0, dsrc1);
    rtl_mv(s, s1, dsrc2);
    *s0 %= 32;
    rtl_shr(s, ddest, ddest, s0);
    rtl_shli(s, s1, s1, 32-*s0);
    rtl_or(s, ddest, ddest, s1);
    operand_write(s, id_dest, ddest);
    print_asm_template2(shrd);
}
