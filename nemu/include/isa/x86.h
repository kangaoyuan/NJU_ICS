#ifndef __ISA_X86_H__
#define __ISA_X86_H__

#include <common.h>

// memory
#define x86_PMEM_BASE 0x0
#define x86_IMAGE_START 0x100000

// reg
/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. */
typedef struct {
    /* Do NOT change the order of the GPRs' definitions. */
    union {
        struct {
            rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
        };
        union {
            uint32_t _32;
            uint16_t _16;
            uint8_t  _8[2];
        } gpr[8];
    };
    vaddr_t pc;
} x86_CPU_state;

// decode
typedef struct {
    bool            is_operand_size_16;
    uint8_t         ext_opcode;
    const rtlreg_t* mbase;
    rtlreg_t        mbr;
    word_t          moff;
} x86_ISADecodeInfo;

#define x86_has_mem_exception() (false)
#define isa_vaddr_check(vaddr, type, len) (MEM_RET_OK)
#define suffix_char(width)                                                 \
    ((width) == 4 ? 'l' : ((width) == 2 ? 'w' : ((width) == 1 ? 'b' : '?')))

#endif
