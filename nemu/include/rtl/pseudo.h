#ifndef __RTL_PSEUDO_H__
#define __RTL_PSEUDO_H__

#include <rtl/rtl.h>

#ifndef __RTL_RTL_H__
#   error "Should be only included by <rtl/rtl.h>"
#endif

/* RTL pseudo instructions */

static inline def_rtl(li, rtlreg_t* dest, const rtlreg_t imm) {
    rtl_addi(s, dest, rz, imm);
}

static inline def_rtl(mv, rtlreg_t* dest, const rtlreg_t* src1) {
    if (dest != src1)
        rtl_add(s, dest, rz, src1);
}

static inline def_rtl(not, rtlreg_t *dest, const rtlreg_t* src1) {
    // dest <- ~src1
    *dest = ~(*src1);
}

static inline def_rtl(neg, rtlreg_t *dest, const rtlreg_t* src1) {
    // dest <- -src1
    *dest = -(*src1);
}

static inline def_rtl(sext, rtlreg_t* dest, const rtlreg_t* src1,
                      int width) {
    // dest <- signext(src1[(width * 8 - 1) .. 0])
    switch (width) {
    case 1:
        *dest = (*src1 >> 7 & 1) == 1 ? *src1 | 0xffffff00 : *src1 & 0x000000ff;
        break;
    case 2:
        *dest = (*src1 >> 15 & 1) == 1 ? *src1 | 0xffff0000 : *src1 & 0x0000ffff;
        break;
    case 4:
        *dest = *src1;
        break;
    default:
        assert(0);
    }
}

static inline def_rtl(zext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- zeroext(src1[(width * 8 - 1) .. 0])
    switch (width) {
    case 1:
        *dest = *(uint8_t*)src1;
        break;
    case 2:
        *dest = *(uint16_t*)src1;
        break;
    case 4:
        *dest = *(uint32_t*)src1;
        break;
    default:
        assert(0);
    }
}

static inline def_rtl(msb, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- src1[width * 8 - 1]
    *dest = *src1 >> (width * 8 - 1) & 1;
}

#endif
