#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int type [[maybe_unused]],
                          int len __attribute__((unused))){
    uint32_t va_dir = vaddr >> 22;
    uint32_t va_tbl = vaddr >> 12 & ((1 << 10) - 1);

    uint32_t pde = paddr_read(cpu.CR3 + va_dir * 4, 4);
    if (!(pde & 1)) {
        printf("MMU pde fail: pde = %x dir_idx = %x base = %x\n", pde, va_dir, cpu.CR3);
        assert(0);
    }
    pde = pde & ~(PAGE_MASK);

    uint32_t pte = paddr_read(pde + va_tbl * 4, 4);
    if (!(pte & 1)) {
        printf("MMU pte fail: pte = %x tbl_idx = %x base = %x\n", pde, va_tbl, pde);
        assert(0);
    }
    return (pte & ~(PAGE_MASK)) + (vaddr & (PAGE_MASK));
}

paddr_t vaddr_read_cross_page(vaddr_t vaddr, int type __attribute__((unused)), int len) {
    uint32_t low = 0, high = 0;
    uint32_t offset = vaddr & 0xfff;
    assert(offset + len > PAGE_SIZE);
    uint32_t pre_len = PAGE_SIZE - offset;
    uint32_t next_len = offset + len - PAGE_SIZE;
    assert((uint32_t)len == pre_len + next_len);

    paddr_t  paddr = isa_mmu_translate(vaddr, type, pre_len);
    for(uint32_t i = 0; i < pre_len; ++i){
        low += paddr_read(paddr+i, 1) << (i * 8); 
    }

    vaddr = (vaddr & (~0xfff)) + PAGE_SIZE;
    paddr = isa_mmu_translate(vaddr, type, next_len);
    for(uint32_t i = 0; i < next_len; ++i){
        high += paddr_read(paddr+i, 1) << (i * 8); 
    }

    return (high << (8 * pre_len)) | low;
}

word_t vaddr_mmu_read(vaddr_t addr, int len, int type) {
    if ((addr & PAGE_MASK) + len > PAGE_SIZE) {
        return vaddr_read_cross_page(addr, type, len);
    } else {
        paddr_t paddr = isa_mmu_translate(addr, type, len);
        //assert(paddr == addr);
        return paddr_read(paddr, len);
    }
}

void vaddr_write_cross_page(vaddr_t vaddr ,word_t data,int len) {
    uint8_t a[4] = {data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff,
                    (data >> 24) & 0xff};
    for (int i = 0; i < len; i++) {
        vaddr_write(isa_mmu_translate(vaddr+i, MEM_TYPE_WRITE, 1), a[i], 1);
    }
}

void vaddr_mmu_write(vaddr_t addr, word_t data, int len){
    if ((addr & PAGE_MASK) + len > PAGE_SIZE) {
        return vaddr_write_cross_page(addr, data, len);
    } else {
        paddr_t paddr = isa_mmu_translate(addr, MEM_TYPE_WRITE, len);
        //assert(paddr == addr);
        return paddr_write(paddr, data, len);
    }
}
