#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int type[[maybe_unused]], int len __attribute__((unused))) {
    uint32_t  va_dir = vaddr >> 22;
    uint32_t  va_pg = vaddr >> 12 & ((1 << 10) - 1);

    uint32_t  pte = paddr_read(cpu.CR3 + va_dir * 4, 4);
    if(!(pte & 1)){
        printf("error to tanslate: vaddr: %x\n", vaddr); 
    }
    assert(pte & 1);
    pte = pte & ~(PAGE_MASK);
    uint32_t  pa_pg = paddr_read(pte + va_pg * 4, 4);
    assert(pa_pg & 1);
    return (pa_pg & ~(PAGE_MASK)) + (vaddr & (PAGE_MASK));
}

word_t vaddr_mmu_read(vaddr_t vaddr, int len, int type){
    paddr_t paddr = isa_mmu_translate(vaddr, type, len);
    assert(vaddr == paddr);

    paddr_t page_end = (vaddr/PAGE_SIZE + 1)*PAGE_SIZE;
    if(vaddr + len > page_end){
        int pre_len = page_end - vaddr; 
        int next_len = len - pre_len; 
        word_t pre_res = 0;
        for(int i = 0; i < pre_len; ++i){
            pre_res = (pre_res << 8) + paddr_read(paddr, 1);
        }
        paddr_t next_paddr = isa_mmu_translate(vaddr+pre_len, type, next_len);
        word_t next_res = 0;
        for(int i = 0; i < next_len; ++i){
            next_res = (next_res << 8) + paddr_read(next_paddr, 1);
        }
        return (next_res << (pre_len*8)) | pre_res;
    }

    return paddr_read(paddr, len);
}

void vaddr_mmu_write(vaddr_t vaddr, word_t data, int len){
    paddr_t paddr = isa_mmu_translate(vaddr, MEM_TYPE_WRITE, len);
    assert(vaddr == paddr);

    paddr_t page_end = (vaddr/PAGE_SIZE + 1)*PAGE_SIZE;
    if(vaddr + len > page_end){
        int pre_len = page_end - vaddr; 
        int next_len = len - pre_len; 
        paddr_write(paddr, data, pre_len);
        paddr_t next_paddr = isa_mmu_translate(vaddr+pre_len, MEM_TYPE_WRITE, next_len);
        paddr_write(next_paddr, data >> (pre_len*8), next_len);
    }

    paddr_write(paddr, data, len);
}
