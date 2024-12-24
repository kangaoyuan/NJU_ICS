#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

#define CR0_PG         0x80000000

paddr_t isa_mmu_translate(vaddr_t vaddr, int type, int len) {
    uint32_t off_page = vaddr & 0xfff;
    uint32_t off_dir = (uint32_t)vaddr >> 22;
    uint32_t off_pte = ((uint32_t)vaddr & 0x003ff000) >> 12;

    paddr_t dir = cpu.CR3;
    paddr_t pte = paddr_read(dir + off_dir * 4, 4);
    if ((pte & 1) == 0) {
        printf("1 error: %08x\n", pte);
        return MEM_RET_FAIL;
    } else {
        paddr_t pagebase = paddr_read((pte & 0xfffff000) + off_pte * 4, 4);

        if ((pagebase & 1) == 0)
            return MEM_RET_FAIL;
        else
            return ((pagebase & 0xfffff000) + off_page);
    }

    return MEM_RET_FAIL;
}

/*
 *int isa_vaddr_check(vaddr_t vaddr, int type, int len){
 *   int vm_open = ((cpu.cr0 & CR0_PG)!=0);
 *  if (vm_open) {
 *    return MEM_RET_NEED_TRANSLATE;
 *  }
 *else{
 *  return MEM_RET_OK;
 *}
 *}
 */



word_t vaddr_mmu_read(vaddr_t vaddr, int len, int type) {
  paddr_t paddr = isa_mmu_translate(vaddr,type,len);
  if(paddr == MEM_RET_FAIL){
    printf("vaddr mmu read %x :fail\n",vaddr);
   // printf("cr0 :%x  cr3: %x\n",cpu.cr0,cpu.cr3);
    assert(0);
  }
  else{
    paddr_t limit = ((vaddr /PAGE_SIZE)+1)*PAGE_SIZE;
    if(vaddr+ len > limit){
      //printf("vaddr mmu read %x :cross_page\n",vaddr);
      word_t res1 = 0;
      word_t res2 = 0;
      int len_in = limit - vaddr;
      int len_over = vaddr+ len - limit;
      uint32_t paddr_in = isa_mmu_translate(vaddr,MEM_TYPE_READ,len_in);
      //res1 = paddr_read(paddr_in,len_in);
        for(int i = 0; i < len_in; ++i){
            res1 = (res1 << 8) + paddr_read(paddr_in, 1);
        }
      uint32_t paddr_over = isa_mmu_translate(vaddr+len_in,MEM_TYPE_READ,len_over);
      //res2 = paddr_read(paddr_over,len_over);
        for(int i = 0; i < len_over; ++i){
            res2 = (res2 << 8) + paddr_read(paddr_over, 1);
        }

      //printf("cross page handled\n");
      return (res2<<(len_in*8))| res1;
      //assert(0);
    }
  }
  return paddr_read(paddr, len);
}


void vaddr_mmu_write(vaddr_t vaddr, word_t data, int len)
{
  paddr_t paddr = isa_mmu_translate(vaddr, MEM_TYPE_WRITE, len);
  if (paddr == MEM_RET_FAIL) {
    printf("vaddr mmu write %x:fail   pc %x\n",vaddr,cpu.pc);
    assert(0);
  }
  else{
    paddr_t limit = ((vaddr /PAGE_SIZE)+1)*PAGE_SIZE;
    if(vaddr+ len > limit){
      //printf("vaddr mmu write %x:cross_page\n",vaddr);

      int len_in = limit - vaddr;
      int len_over = vaddr+ len - limit;
      uint32_t paddr_in = isa_mmu_translate(vaddr,MEM_TYPE_READ,len_in);
       uint32_t paddr_over = isa_mmu_translate(vaddr+len_in,MEM_TYPE_READ,len_over);
        paddr_write(paddr_in,data,len_in);
        paddr_write(paddr_over,data>>(len_in*8),len_over);
      }
  }

  paddr_write(paddr, data, len);
}
/*
 *paddr_t isa_mmu_translate(vaddr_t vaddr, int type [[maybe_unused]],
 *                          int len __attribute__((unused))){
 *    uint32_t va_dir = vaddr >> 22;
 *    uint32_t va_pg = vaddr >> 12 & ((1 << 10) - 1);
 *
 *    uint32_t pte = paddr_read(cpu.CR3 + va_dir * 4, 4);
 *    if (!(pte & 1)) {
 *        printf("error to tanslate: vaddr: %x\n", vaddr);
 *    }
 *    assert(pte & 1);
 *    pte = pte & ~(PAGE_MASK);
 *    uint32_t pa_pg = paddr_read(pte + va_pg * 4, 4);
 *    assert(pa_pg & 1);
 *    return (pa_pg & ~(PAGE_MASK)) + (vaddr & (PAGE_MASK));
 *}
 *
 *word_t vaddr_mmu_read(vaddr_t vaddr, int len, int type){
 *    printf("vaddr_mmu_read, vaddr == %x\n", vaddr);
 *    paddr_t paddr = isa_mmu_translate(vaddr, type, len);
 *    assert(vaddr == paddr);
 *
 *    paddr_t page_end = (vaddr/PAGE_SIZE + 1)*PAGE_SIZE;
 *    if(vaddr + len > page_end){
 *        printf("vaddr_mmu_read across, vaddr == %x page_end == %x\n", vaddr, page_end);
 *        int pre_len = page_end - vaddr; 
 *        int next_len = len - pre_len; 
 *        paddr_t paddr = isa_mmu_translate(vaddr, type, pre_len);
 *        word_t pre_res = 0;
 *        for(int i = 0; i < pre_len; ++i){
 *            pre_res = (pre_res << 8) + paddr_read(paddr, 1);
 *        }
 *        paddr_t next_paddr = isa_mmu_translate(vaddr+pre_len, type, next_len);
 *        word_t next_res = 0;
 *        for(int i = 0; i < next_len; ++i){
 *            next_res = (next_res << 8) + paddr_read(next_paddr, 1);
 *        }
 *        return (next_res << (pre_len*8)) | pre_res;
 *    }
 *
 *    return paddr_read(paddr, len);
 *}
 *
 *void vaddr_mmu_write(vaddr_t vaddr, word_t data, int len){
 *    printf("vaddr_mmu_write, vaddr == %x\n", vaddr);
 *    paddr_t paddr = isa_mmu_translate(vaddr, MEM_TYPE_WRITE, len);
 *    assert(vaddr == paddr);
 *
 *    paddr_t page_end = (vaddr/PAGE_SIZE + 1)*PAGE_SIZE;
 *    if(vaddr + len > page_end){
 *        int pre_len = page_end - vaddr; 
 *        int next_len = len - pre_len; 
 *        paddr_write(paddr, data, pre_len);
 *        paddr_t next_paddr = isa_mmu_translate(vaddr+pre_len, MEM_TYPE_WRITE, next_len);
 *        paddr_write(next_paddr, data >> (pre_len*8), next_len);
 *    }
 *
 *    paddr_write(paddr, data, len);
 *}
 */
