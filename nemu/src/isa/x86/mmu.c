#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

#define PT_P 1
paddr_t isa_mmu_translate(vaddr_t vaddr, int type, int len)
{
  uint32_t va_dir_idx = ((uint32_t)vaddr >> 22) & 0x3ff;
  uint32_t va_page_table_idx = ((uint32_t)vaddr >> 12) & 0x3ff;
  uint32_t offset = vaddr & 0xfff;
  uint32_t page_dir_base = cpu.CR3&(~0xfff);
  uint32_t page_table_entry = paddr_read(page_dir_base + 4 * va_dir_idx, 4);
  if ((page_table_entry & PT_P) == 0)
  {
    printf("MMU pte fail:pte = %x pdx = %x base = %x\n",page_table_entry,va_dir_idx,cpu.CR3);
    return MEM_RET_FAIL;
  }
  else
  {
    uint32_t page_table_value = paddr_read((page_table_entry & (~0xfff)) + 4 * va_page_table_idx, 4);
    if ((page_table_value & PT_P) == 0) 
    {
      printf("MMU:page table value: %x PTE:%x base = %x pc = %x\n",page_table_value,page_table_entry&(~0xfff),cpu.CR3,cpu.pc);
      return MEM_RET_FAIL;
    }
    else
    {
      if (offset + len > PAGE_SIZE)
      {
        //printf("%d %d\n",len,offset);
        return MEM_RET_CROSS_PAGE;
      }
      else return MEM_RET_OK;
    }
  }
}

/*
 *paddr_t isa_mmu_translate(vaddr_t vaddr, int type[[maybe_unused]], int len __attribute__((unused))) {
 *    uint32_t  va_dir = vaddr >> 22;
 *    uint32_t  va_pg = vaddr >> 12 & ((1 << 10) - 1);
 *
 *    uint32_t  pte = paddr_read(cpu.CR3 + va_dir * 4, 4);
 *    if(!(pte & 1)){
 *        printf("error to tanslate: vaddr: %x\n", vaddr); 
 *    }
 *    assert(pte & 1);
 *    pte = pte & ~(PAGE_MASK);
 *    uint32_t  pa_pg = paddr_read(pte + va_pg * 4, 4);
 *    assert(pa_pg & 1);
 *    return (pa_pg & ~(PAGE_MASK)) + (vaddr & (PAGE_MASK));
 *}
 */

word_t vaddr_mmu_read(vaddr_t vaddr, int len, int type){
    printf("vaddr_mmu_read, vaddr == %x\n", vaddr);
    paddr_t paddr = isa_mmu_translate(vaddr, type, len);
    assert(vaddr == paddr);

    paddr_t page_end = (vaddr/PAGE_SIZE + 1)*PAGE_SIZE;
    if(vaddr + len > page_end){
        printf("vaddr_mmu_read across, vaddr == %x page_end == %x\n", vaddr, page_end);
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
    printf("vaddr_mmu_write, vaddr == %x\n", vaddr);
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
