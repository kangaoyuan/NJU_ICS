#include <time.h>
#include <stdlib.h>
#include <isa.h>
#include <device/map.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

static uint8_t pmem[PMEM_SIZE] PG_ALIGN = {};

// The void* guest_to_host(paddr_t); function maps the memory address(arg) that the CPU will access to the corresponding offset in pmem.
void* guest_to_host(paddr_t paddr) {
    return pmem + (paddr - PMEM_BASE);
}
paddr_t host_to_guest(void* haddr) {
    return haddr - (void*)pmem + PMEM_BASE;
}

void init_mem() {
#ifndef DIFF_TEST
    srand(time(NULL));
    uint32_t* p = (uint32_t*)pmem;
    for (int i = 0; i < PMEM_SIZE / sizeof(p[0]); i++)
        p[i] = rand();
#endif
}

static inline bool in_pmem(paddr_t addr) {
  return (PMEM_BASE <= addr) && (addr < PMEM_BASE + PMEM_SIZE);
}

static inline word_t pmem_read(paddr_t addr, int len) {
    void* p = &pmem[addr - PMEM_BASE];
    switch (len) {
    case 1:
        return *(uint8_t*)p;
    case 2:
        return *(uint16_t*)p;
    case 4:
        return *(uint32_t*)p;
#ifdef ISA64
    case 8:
        return *(uint64_t*)p;
#endif
    default:
        assert(0);
    }
}

static inline void pmem_write(paddr_t addr, word_t data, int len) {
    void* p = &pmem[addr - PMEM_BASE];
    switch (len) {
    case 1:
        *(uint8_t*)p = data;
        return;
    case 2:
        *(uint16_t*)p = data;
        return;
    case 4:
        *(uint32_t*)p = data;
        return;
#ifdef ISA64
    case 8:
        *(uint64_t*)p = data;
        return;
#endif
    default:
        assert(0);
    }
}

/* Memory accessing interfaces */

IOMap* fetch_mmio_map(paddr_t addr);
word_t vaddr_mmu_read(vaddr_t addr, int len, int type);
void vaddr_mmu_write(vaddr_t addr, word_t data, int len);

inline word_t paddr_read(paddr_t addr, int len) {
    if (in_pmem(addr))
        return pmem_read(addr, len);
    else
        return map_read(addr, len, fetch_mmio_map(addr));
}

inline void paddr_write(paddr_t addr, word_t data, int len) {
    if (in_pmem(addr))
        pmem_write(addr, data, len);
    else
        map_write(addr, data, len, fetch_mmio_map(addr));
}


#define def_vaddr_template(bytes)                                          \
    word_t concat(vaddr_ifetch, bytes)(vaddr_t addr) {                     \
        int ret = isa_vaddr_check(addr, MEM_TYPE_IFETCH, bytes);           \
        if (ret == MEM_RET_OK)                                             \
            return paddr_read(addr, bytes);                                \
        return 0;                                                          \
    }                                                                      \
    word_t concat(vaddr_read, bytes)(vaddr_t addr) {                       \
        int ret = isa_vaddr_check(addr, MEM_TYPE_READ, bytes);             \
        if (ret == MEM_RET_OK)                                             \
            return paddr_read(addr, bytes);                                \
        return 0;                                                          \
    }                                                                      \
    void concat(vaddr_write, bytes)(vaddr_t addr, word_t data) {           \
        int ret = isa_vaddr_check(addr, MEM_TYPE_WRITE, bytes);            \
        if (ret == MEM_RET_OK)                                             \
            paddr_write(addr, data, bytes);                                \
    }

def_vaddr_template(1)
def_vaddr_template(2)
def_vaddr_template(4)
#ifdef ISA64
def_vaddr_template(8)
#endif
