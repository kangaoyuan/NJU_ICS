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
    union {
        struct {
            uint32_t CF : 1;
            uint32_t resv1 : 1;
            uint32_t PF : 1;
            uint32_t resv2 : 1;
            uint32_t AF : 1;
            uint32_t resv3 : 1;
            uint32_t ZF : 1;
            uint32_t SF : 1;
            uint32_t TF : 1;
            uint32_t IF : 1;
            uint32_t DF : 1;
            uint32_t OF : 1;
            uint32_t IOPL : 2;
            uint32_t NT : 1;
            uint32_t resv4 : 1;
            uint32_t RF : 1;
            uint32_t VM : 1;
            uint32_t resv5 : 14;
        };
        uint32_t val;
    } eflags;

    uint32_t cs;
	uint32_t ss;

	struct{
		uint32_t idtr_base;
		uint16_t idtr_limit;
	};

	struct{
		uint32_t base;
		uint16_t limit;
	}gdtr;

	uint32_t CR0;
	uint32_t CR3;

	uint16_t TR;

	bool INTR;
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
