#ifndef __X86_RTL_H__
#define __X86_RTL_H__

#include "reg.h"
#include <rtl/rtl.h>
#include <rtl/pseudo.h>

/* RTL pseudo instructions */

static inline def_rtl(lr, rtlreg_t* dest, int r, int width) {
    switch (width) {
    case 4:
        rtl_mv(s, dest, &reg_l(r));
        return;
    case 1:
        rtl_host_lm(s, dest, &reg_b(r), 1);
        return;
    case 2:
        rtl_host_lm(s, dest, &reg_w(r), 2);
        return;
    default:
        assert(0);
    }
}

static inline def_rtl(sr, int r, const rtlreg_t* src1, int width) {
    switch (width) {
    case 4:
        rtl_mv(s, &reg_l(r), src1);
        return;
    case 1:
        rtl_host_sm(s, &reg_b(r), src1, 1);
        return;
    case 2:
        rtl_host_sm(s, &reg_w(r), src1, 2);
        return;
    default:
        assert(0);
    }
}

static inline def_rtl(push, const rtlreg_t* src1) {
    // esp <- esp - 4
    // M[esp] <- src1
    // TODO();
    rtl_mv(s, t0, src1);
    rtl_subi(s, &cpu.esp, &cpu.esp, 4);
    rtl_sm(s, &cpu.esp, 0, t0, 4);
}

static inline def_rtl(pop, rtlreg_t* dest) {
    // dest <- M[esp]
    // esp <- esp + 4
    // TODO();
    rtl_lm(s, dest, &cpu.esp, 0, 4);
    rtl_addi(s, &cpu.esp, &cpu.esp, 4);
}

static inline def_rtl(is_sub_overflow, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
    // dest <- is_overflow(src1 - src2)
    // TODO();
    switch (width) {
    case 1:
        if (((int8_t)(*src1) < (int8_t)(*src2) && (int8_t)(*res) > 0) ||
            ((int8_t)(*src1) > (int8_t)(*src2) && (int8_t)(*res) < 0)) {
            *dest = true;
        } else {
            *dest = false;
        }
        break;
    case 2:
        if (((int16_t)(*src1) < (int16_t)(*src2) && (int16_t)(*res) > 0) ||
            ((int16_t)(*src1) > (int16_t)(*src2) && (int16_t)(*res) < 0)) {
            *dest = true;
        } else {
            *dest = false;
        }
        break;
    case 4:
        if (((int32_t)(*src1) < (int32_t)(*src2) && (int32_t)(*res) > 0) ||
            ((int32_t)(*src1) > (int32_t)(*src2) && (int32_t)(*res) < 0)) {
            *dest = true;
        } else {
            *dest = false;
        }
        break;
    default:
        assert(0);
    }
}

static inline def_rtl(is_sub_carry, rtlreg_t* dest,
    const rtlreg_t* src1, const rtlreg_t* src2) {
    // dest <- is_carry(src1 - src2)
    // TODO();
    rtl_setrelop(s, RELOP_LTU, dest, src1, src2);
}

static inline def_rtl(is_add_overflow, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
    // dest <- is_overflow(src1 + src2)
    // TODO();
    switch (width) {
    case 1:
        if (((int8_t)(*src1) > 0 && (int8_t)(*src2) > 0 &&
             (int8_t)(*res) < 0) ||
            ((int8_t)(*src1) < 0 && (int8_t)(*src2) < 0 &&
             (int8_t)(*res) > 0)) {
            *dest = true;
        } else {
            *dest = false;
        }
        break;
    case 2:
        if (((int16_t)(*src1) > 0 && (int16_t)(*src2) > 0 &&
             (int16_t)(*res) < 0) ||
            ((int16_t)(*src1) < 0 && (int16_t)(*src2) < 0 &&
             (int16_t)(*res) > 0)) {
            *dest = true;
        } else {
            *dest = false;
        }
        break;
    case 4:
        if (((int32_t)(*src1) > 0 && (int32_t)(*src2) > 0 &&
             (int32_t)(*res) < 0) ||
            ((int32_t)(*src1) < 0 && (int32_t)(*src2) < 0 &&
             (int32_t)(*res) > 0)) {
            *dest = true;
        } else {
            *dest = false;
        }
        break;
    default:
        assert(0);
    }
}

static inline def_rtl(is_add_carry, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) {
    // dest <- is_carry(src1 + src2)
    // TODO();
    rtl_setrelop(s, RELOP_LTU, dest, res, src1);
}

#define def_rtl_setget_eflags(f) \
  static inline def_rtl(concat(set_, f), const rtlreg_t* src) { \
      cpu.eflags.f = *src;\
  } \
  static inline def_rtl(concat(get_, f), rtlreg_t* dest) { \
      *dest = cpu.eflags.f;\
  }

def_rtl_setget_eflags(CF)
def_rtl_setget_eflags(OF)
def_rtl_setget_eflags(ZF)
def_rtl_setget_eflags(SF)

static inline def_rtl(update_ZF, const rtlreg_t* result, int width) {
    // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
    //TODO();
    switch (width) {
    case 1:
        *t0 = ((*result & 0xff) == 0);
        rtl_set_ZF(s, t0);
        break;
    case 2:
        *t0 = ((*result & 0xffff) == 0);
        rtl_set_ZF(s, t0);
        break;
    case 4:
        *t0 = (*result == 0);
        rtl_set_ZF(s, t0);
        break;
    default:
        assert(0);
    }
    rtl_set_ZF(s, t0);
}

static inline def_rtl(update_SF, const rtlreg_t* result, int width) {
    // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
    //TODO();
    rtl_msb(s, t0, result, width);
    rtl_set_SF(s, t0);
}

static inline def_rtl(update_ZFSF, const rtlreg_t* result, int width) {
    rtl_update_ZF(s, result, width);
    rtl_update_SF(s, result, width);
}

#endif
