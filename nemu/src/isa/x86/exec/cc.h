#ifndef __CC_H__
#define __CC_H__

#include "../local-include/rtl.h" // IWYU pragma: keep

enum {
  CC_O, CC_NO, CC_B,  CC_NB,
  CC_E, CC_NE, CC_BE, CC_NBE,
  CC_S, CC_NS, CC_P,  CC_NP,
  CC_L, CC_NL, CC_LE, CC_NLE
};


/* Condition Code */

static inline const char* get_cc_name(int subcode) {
    static const char* cc_name[] = {"o",  "no",  "b",  "nb", "e", "ne",
                                    "be", "nbe", "s",  "ns", "p", "np",
                                    "l",  "nl",  "le", "nle"};
    return cc_name[subcode];
}

static inline void rtl_setcc(DecodeExecState* s, rtlreg_t* dest,
                             uint32_t subcode) {
    uint32_t invert = subcode & 0x1;

    // TODO: Query EFLAGS to determine whether the condition code is
    // satisfied. dest <- ( cc is satisfied ? 1 : 0)
    switch (subcode & 0xe) {
    case CC_O:
        rtl_get_OF(s, dest);
        break;
    case CC_B:
        rtl_get_CF(s, dest);
        break;
    case CC_E:
        rtl_get_ZF(s, dest);
        break;
    case CC_BE:
        rtl_get_CF(s, t0);
        rtl_get_ZF(s, dest);
        rtl_or(s, dest, t0, dest);
        break;
    case CC_S:
        rtl_get_SF(s, dest);
        break;
    case CC_L:
        rtl_get_SF(s, t0);
        rtl_get_OF(s, dest);
        rtl_xor(s, dest, t0, dest);
        break;
    case CC_LE:
<<<<<<< HEAD
        rtl_get_SF(s, t0);
        rtl_get_OF(s, dest);
        rtl_xor(s, dest, t0, dest);
        rtl_get_ZF(s, t0);
        rtl_or(s, dest, dest, t0);
        break;
=======
        TODO();
>>>>>>> 0a92b12 (>  compile)
    case CC_P:
        panic("PF is not supported");
    default:
        panic("should not reach here");
    }

    if (invert) {
        rtl_xori(s, dest, dest, 0x1);
    }
    assert(*dest == 0 || *dest == 1);
}

#endif
