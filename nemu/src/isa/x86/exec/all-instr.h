#include "../local-include/rtl.h"
#include "../local-include/decode.h"

#include "arith.h"      // cmp add sub inc dec neg mul div
#include "control.h"    // jcc jmp call ret
#include "data-mov.h"   // lea mov push pop leave
#include "logic.h"      // test setcc and or xor not shl sar
#include "system.h"     // in out int iret lidt mov_r2cr mov_cr2r
#include "string.h"

def_EHelper(nop);
def_EHelper(inv);
def_EHelper(nemu_trap);
