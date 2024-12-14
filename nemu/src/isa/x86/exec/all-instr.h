#include "../local-include/rtl.h"       // IWYU pragma: keep
#include "../local-include/decode.h"    // IWYU pragma: keep

#include "control.h"    // jcc jmp call ret
#include "arith.h"      // cmp add sub inc dec neg mul div
#include "data-mov.h"   // lea mov push pop leave cltd cwtl
#include "system.h"     // in out int iret lidt mov_r2cr mov_cr2r
#include "logic.h"      // test setcc and or xor not shl sar bsr shld shrd

def_EHelper(nop);
def_EHelper(inv);
def_EHelper(nemu_trap);
