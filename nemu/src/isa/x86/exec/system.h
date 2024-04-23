#include <monitor/difftest.h>
#include "../local-include/decode.h"

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);
void raise_intr(DecodeExecState *s, uint32_t NO, vaddr_t ret_addr);

static inline def_EHelper(lidt) {
    //TODO();
    vaddr_t addr = *s->isa.mbase + s->isa.moff;
    cpu.idtr_limit = vaddr_read(addr, 2);
    cpu.idtr_base = vaddr_read(addr+2, 4);
    printf("lidt execution, idt == %x\n", cpu.idtr_base);
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

static inline def_EHelper(int) {
    // TODO();
    raise_intr(s, *ddest, s->seq_pc);
    print_asm("int %s", id_dest->str);

#ifndef __DIFF_REF_NEMU__
    difftest_skip_dut(1, 2);
#endif
}

static inline def_EHelper(iret) {
    // TODO();
    rtl_pop(s, &cpu.pc);
    rtl_pop(s, &cpu.cs);
    rtl_pop(s, &cpu.eflags.val);
    rtl_j(s, cpu.pc);
    print_asm("iret");

#ifndef __DIFF_REF_NEMU__
    difftest_skip_ref();
#endif
}

static inline def_EHelper(in) {
    switch (id_src1->width) {
    case 1:
        *s0 = pio_read_b(*dsrc1);
        break;
    case 2:
        *s0 = pio_read_w(*dsrc1);
        break;
    case 4:
        *s0 = pio_read_l(*dsrc1);
        break;
    default:
        panic("Invalid width");
    }
    operand_write(s, id_dest, s0);
    print_asm_template2(in);
}

static inline def_EHelper(out) {
    switch (id_src1->width) {
    case 1:
        pio_write_b(*ddest, *dsrc1);
        break;
    case 2:
        pio_write_w(*ddest, *dsrc1);
        break;
    case 4:
        pio_write_l(*ddest, *dsrc1);
        break;
    default:
        panic("Invalid width");
    }
    print_asm_template2(out);
}
