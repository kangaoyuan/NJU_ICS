#include "../local-include/decode.h"

static inline def_EHelper(lea) {
  rtl_addi(s, ddest, s->isa.mbase, s->isa.moff);
  operand_write(s, id_dest, ddest);
  print_asm_template2(lea);
}

static inline def_EHelper(mov) {
    operand_write(s, id_dest, dsrc1);
    print_asm_template2(mov);
}

static inline def_EHelper(push) {
    rtl_push(s, ddest);
    print_asm_template1(push);
}

static inline def_EHelper(pop) {
    rtl_pop(s, s0);
    operand_write(s, id_dest, s0);
    print_asm_template1(pop);
}

static inline def_EHelper(pusha) {
    if (s->isa.is_operand_size_16) {
        rtlreg_t temp = reg_w(R_SP);
        rtl_push(s, (const rtlreg_t*)&reg_w(R_AX));
        rtl_push(s, (const rtlreg_t*)&reg_w(R_CX));
        rtl_push(s, (const rtlreg_t*)&reg_w(R_DX));
        rtl_push(s, (const rtlreg_t*)&reg_w(R_BX));
        rtl_push(s, &temp);
        rtl_push(s, (const rtlreg_t*)&reg_w(R_BP));
        rtl_push(s, (const rtlreg_t*)&reg_w(R_SI));
        rtl_push(s, (const rtlreg_t*)&reg_w(R_DI));
    } else {
        rtlreg_t temp = cpu.esp;
        rtl_push(s, &cpu.eax);
        rtl_push(s, &cpu.ecx);
        rtl_push(s, &cpu.edx);
        rtl_push(s, &cpu.ebx);
        rtl_push(s, &temp);
        rtl_push(s, &cpu.ebp);
        rtl_push(s, &cpu.esi);
        rtl_push(s, &cpu.edi);
    }
    print_asm("pusha");
}

static inline def_EHelper(popa) {
    rtl_pop(s, &cpu.edi);
    rtl_pop(s, &cpu.esi);
    rtl_pop(s, &cpu.ebp);
    rtl_pop(s, s0);
    rtl_pop(s, &cpu.ebx);
    rtl_pop(s, &cpu.edx);
    rtl_pop(s, &cpu.ecx);
    rtl_pop(s, &cpu.eax);
    print_asm("popa");
}

static inline def_EHelper(leave) {
    rtl_mv(s, &reg_l(R_ESP), &reg_l(R_EBP));
    rtl_pop(s, &reg_l(R_EBP));
    print_asm("leave");
}

static inline def_EHelper(cltd) {
    if (s->isa.is_operand_size_16) {
        rtl_msb(s, s0, (rtlreg_t*)&reg_w(R_AX), 2);
        rtl_sext(s, (rtlreg_t*)&reg_w(R_DX), s0, 4);
    } else {
        rtl_msb(s, s0, (rtlreg_t*)&reg_l(R_EAX), 4);
        rtl_sext(s, (rtlreg_t*)&reg_l(R_EDX), s0, 4);
    }
    print_asm(s->isa.is_operand_size_16 ? "cwtl" : "cltd");
}

static inline def_EHelper(cwtl) {
    if (s->isa.is_operand_size_16) {
        rtl_sext(s, (rtlreg_t*)&reg_w(R_AX), (rtlreg_t*)&reg_b(R_AL), 2);
    } else {
        rtl_sext(s, (rtlreg_t*)&reg_l(R_EAX), (rtlreg_t*)&reg_w(R_AX), 4);
    }
    print_asm(s->isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

static inline def_EHelper(movzx) {
    rtl_zext(s, ddest, dsrc1, id_dest->width);
    operand_write(s, id_dest, ddest);
    print_asm_template2(movzx);
}

static inline def_EHelper(movsx) {
    rtl_sext(s, ddest, dsrc1, id_src1->width);
    operand_write(s, id_dest, ddest);
    print_asm_template2(movsx);
}

static inline def_EHelper(movs) {
    rtl_lm(s, s0, dsrc1, 0, id_src1->width);
    rtl_sm(s, ddest, 0, s0, id_dest->width);
    operand_write(s, id_dest, ddest);
    rtl_addi(s, dsrc1, dsrc1, 1);
    rtl_addi(s, ddest, ddest, 1); 
    print_asm_template1(movs);
}
