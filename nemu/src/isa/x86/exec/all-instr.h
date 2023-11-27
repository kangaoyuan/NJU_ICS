#include "../local-include/rtl.h"
#include "../local-include/decode.h"

#include "arith.h"      // cmp add sub mul div
#include "control.h"    // jcc jmp call ret
#include "data-mov.h"   // mov push pop lea
#include "logic.h"      // test and or xor shl sar
#include "system.h"
#include "string.h"

def_EHelper(nop);
def_EHelper(inv);
def_EHelper(nemu_trap);
