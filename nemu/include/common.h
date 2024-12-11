#ifndef __COMMON_H__
#define __COMMON_H__

#define DEBUG
//#define DIFF_TEST

#ifdef __ISA_riscv64__
# define ISA64
#endif

#if _SHARE
// do not enable these features while building a reference design
#undef DIFF_TEST
#undef DEBUG
#endif

/* You will define this macro in PA2 */
#define HAS_IOE

#include <stdint.h>
#include <assert.h> // IWYU pragma: keep
#include <string.h>// IWYU pragma: keep

//below doesn't help, so using Include What You Use // IWYU pragma: keep
// or 
//#pragma clang diagnostic ignored "-Wno-unused-include"

#ifdef ISA64
typedef uint64_t word_t;
typedef int64_t  sword_t;
#define FMT_WORD "0x%016lx"
#else
typedef uint32_t word_t;
typedef int32_t  sword_t;
#define FMT_WORD "0x%08x"
#endif

typedef word_t   rtlreg_t;
typedef word_t   vaddr_t;
typedef uint32_t paddr_t;
typedef uint16_t ioaddr_t;

#define true 1
#define false 0
typedef uint8_t bool;


#include <debug.h>
#include <macro.h>

#endif // __COMMON_H__
