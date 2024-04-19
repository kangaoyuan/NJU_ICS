#include "../local-include/rtl.h"       // IWYU pragma: keep
#include "../local-include/decode.h"    // IWYU pragma: keep

#include "arith.h"      // cmp add sub inc dec neg mul div
#include "control.h"    // jcc jmp call ret
#include "data-mov.h"   // lea mov push pop leave
#include "logic.h"      // test setcc and or xor not shl sar
#include "system.h"     // in out int iret lidt mov_r2cr mov_cr2r

def_EHelper(nop);
def_EHelper(inv);
def_EHelper(nemu_trap);
