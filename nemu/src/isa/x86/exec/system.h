#include "../local-include/decode.h"
#include <monitor/difftest.h>

static inline def_EHelper(lidt) {
    rtl_shri(s, ddest, ddest, 16);
    cpu.idtr_limit = *ddest;
    cpu.idtr_base = vaddr_read(*ddest + 2, 4);
    print_asm_template1(lidt);
}

static inline def_EHelper(mov_r2cr) {
  TODO();
  print_asm("movl %%%s,%%cr%d", reg_name(id_src1->reg, 4), id_dest->reg);
}

static inline def_EHelper(mov_cr2r) {
  TODO();
  print_asm("movl %%cr%d,%%%s", id_src1->reg, reg_name(id_dest->reg, 4));

#ifndef __DIFF_REF_NEMU__
  difftest_skip_ref();
#endif
}

void raise_intr(DecodeExecState* s, word_t NO, vaddr_t ret_addr);
static inline def_EHelper(int) {
    raise_intr(s, *ddest, s->seq_pc);
    print_asm("int %s", id_dest->str);

#ifndef __DIFF_REF_NEMU__
    difftest_skip_dut(1, 2);
#endif
}

static inline def_EHelper(iret) {
    rtl_pop(s, s0);
    rtl_pop(s, &cpu.cs);
    rtl_pop(s, &cpu.eflags.val);
    rtl_j(s, *s0);
    print_asm("iret");

#ifndef __DIFF_REF_NEMU__
    difftest_skip_ref();
#endif
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);
static inline def_EHelper(in) {
    switch (id_dest->width) {
    case 1:
        *s0 = pio_read_b(*dsrc1);
        break;
    case 2:
        *s0 = pio_read_w(*dsrc1);
        break;
    case 4:
        *s0 = pio_read_l(*dsrc1);
        break;
    } 
  //  Warning("in %x:%x -> @%s",id_src1->val, *s0, id_dest->str);
    operand_write(s, id_dest, s0);
    print_asm_template2(in);
}

static inline def_EHelper(out) {
    switch (id_dest->width) {
    case 1:
        pio_write_b(*ddest, *dsrc1);
        break;
    case 2:
        pio_write_w(*ddest, *dsrc1);
        break;
    case 4:
        pio_write_l(*ddest, *dsrc1);
        break;
    }
  //  Warning("out %x -> @%x",id_src1->val,id_dest->val);
    print_asm_template2(out);
}
