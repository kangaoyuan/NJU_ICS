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
  TODO();
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(sar);
}

static inline def_EHelper(shl) {
  TODO();
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shl);
}

static inline def_EHelper(shr) {
  TODO();
  // unnecessary to update CF and OF in NEMU
  print_asm_template2(shr);
}
