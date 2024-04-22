#include "../local-include/decode.h"

static inline def_EHelper(mov) {
    operand_write(s, id_dest, dsrc1);
    print_asm_template2(mov);
}

static inline def_EHelper(push) {
    // TODO();
    rtl_push(s, ddest);
    print_asm_template1(push);
}

static inline def_EHelper(pop) {
    //TODO();
    rtl_pop(s, s0);
    operand_write(s, id_dest, s0);
    print_asm_template1(pop);
}

static inline def_EHelper(pusha) {
  TODO();
  print_asm("pusha");
}

static inline def_EHelper(popa) {
  TODO();
  print_asm("popa");
}

static inline def_EHelper(leave) {
    // The reverse action of push ebp; mov esp ebp;
    // TODO();
    rtl_mv(s, &cpu.esp, &cpu.ebp);
    rtl_pop(s, &cpu.ebp);
    print_asm("leave");
}

static inline def_EHelper(cltd) {
    if (s->isa.is_operand_size_16) {
        //TODO();
        reg_w(R_DX) = (reg_w(R_AX)>>15 & 1) ? 0xffff : 0;
    } else {
        //TODO();
        cpu.edx = (cpu.eax >> 31 & 1) ? 0xffffffff : 0;
    }
    print_asm(s->isa.is_operand_size_16 ? "cwtl" : "cltd");
}

static inline def_EHelper(cwtl) {
    if (s->isa.is_operand_size_16) {
        // TODO();
        reg_w(R_AX) = *(int8_t*)(&reg_b(R_AL));
    } else {
        // TODO();
        reg_l(R_EAX) = *(int16_t*)(&reg_w(R_AX));
    }
    print_asm(s->isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

static inline def_EHelper(movsx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(s, ddest, dsrc1, id_src1->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(movsx);
}

static inline def_EHelper(movzx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  operand_write(s, id_dest, dsrc1);
  print_asm_template2(movzx);
}

static inline def_EHelper(lea) {
    rtl_addi(s, ddest, s->isa.mbase, s->isa.moff);
    operand_write(s, id_dest, ddest);
    print_asm_template2(lea);
}

static inline def_EHelper(movsb) {
    rtl_lm(s, s0, &cpu.esi, 0, 1);
    rtl_sm(s, &cpu.edi, 0, s0, 1);
    rtl_addi(s, &cpu.esi, &cpu.esi, 1);
    rtl_addi(s, &cpu.edi, &cpu.edi, 1);
    print_asm_template1(movs);
}

static inline def_EHelper(movsl) {
    switch (id_dest->width) {
    case 2:
        rtl_lms(s, s0, &cpu.esi, 0, 2);
        rtl_sm(s, &cpu.edi, 0, s0, 2);
        *s1 = 2;
        rtl_add(s, &cpu.esi, &cpu.esi, s1);
        rtl_add(s, &cpu.edi, &cpu.edi, s1);
        break;
    case 4:
        rtl_lms(s, s0, &cpu.esi, 0, 4);
        rtl_sm(s, &cpu.edi, 0, s0, 4);
        *s1 = 4;
        rtl_add(s, &cpu.esi, &cpu.esi, s1);
        rtl_add(s, &cpu.edi, &cpu.edi, s1);
        break;
    }
    print_asm_template1(movl);
}
