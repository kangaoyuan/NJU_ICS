#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

#define PT_P 0x1

paddr_t page_table_walk(vaddr_t vaddr){
    uint32_t va_pd = (vaddr >> 22);
    uint32_t va_pg = (vaddr >> 12) & ((1 << 10) - 1);

    uint32_t pde = paddr_read(cpu.CR3 + 4 * va_pd, 4);
    assert(pde & 0x1);
    uint32_t pte = paddr_read((pde & ~PAGE_MASK) + 4 * va_pg, 4);
    assert(pte & 0x1);
    return (pte & ~PAGE_MASK) | (vaddr & PAGE_MASK);
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int type, int len){
    uint32_t va_pd = (vaddr >> 22);
    uint32_t va_pg = (vaddr >> 12) & ((1 << 10) - 1);

    uint32_t pde = paddr_read(cpu.CR3 + 4 * va_pd, 4);
    if ((pde & PT_P) == 0) {
        printf("MMU pte fail:pte = %x pdx = %x base = %x\n",
               pde, va_pd, cpu.CR3);
        return MEM_RET_FAIL;
    }

    uint32_t pte = paddr_read((pde & (~0xfff)) + 4 * va_pg, 4);
    if ((pte & PT_P) == 0) {
        printf("MMU:page table value: %x PTE:%x base = %x pc = %x\n", pde,
               pte & (~0xfff), cpu.CR3, cpu.pc);
        return MEM_RET_FAIL;
    }

    // Below is crucial
    if ((vaddr & PAGE_MASK) + len > PAGE_SIZE) {
        // printf("%d %d\n",len,offset);
        return MEM_RET_CROSS_PAGE;
    } else
        return MEM_RET_OK;
}

paddr_t vaddr_read_cross_page(vaddr_t vaddr ,int type,int len){
  paddr_t paddr = page_table_walk(vaddr);
  uint32_t offset = vaddr&0xfff;
  uint32_t partial = offset + len - PAGE_SIZE;
  uint32_t low=0,high =0;
  if(len - partial == 3)
  {
    low = paddr_read(paddr,4)&0xffffff;
  }
  else low = paddr_read(paddr,len - partial);
  if(partial == 3)
  {
    high = paddr_read(page_table_walk((vaddr&(~0xfff)) + PAGE_SIZE),4)&0xffffff;
  }
  else high = paddr_read(page_table_walk((vaddr&(~0xfff)) + PAGE_SIZE),partial);
  //printf("pc = %x:offset = %d base = %x :cross read = %x partial = %d, high = %x, low = %x\n",cpu.pc,offset,cpu.CR3,((high << 8*(len-partial))|low),partial,high,low);
  /* assert(len - partial != 3&&partial != 3);
  low = paddr_read(paddr,len - partial);
  high = paddr_read(page_table_walk((vaddr&0xfff) + PAGE_SIZE),partial); */
  //printf("cross read %x\n",((high << 8*(len-partial))|low));
  return ((high << 8*(len-partial))|low);
}

word_t vaddr_mmu_read(vaddr_t addr, int len, int type){
  paddr_t pg_base = isa_mmu_translate(addr,type,len);
  if(pg_base == MEM_RET_OK) {
    paddr_t paddr = page_table_walk(addr);
    assert(addr == paddr);
    //word_t ret = paddr_read(paddr,len);
    //if(len == 4&& (ret&0x80000) == 0x80000 && (ret&0xbfff0000)!=0xbfff0000) printf("read %x %x\n",addr,ret);
    return paddr_read(paddr,len);
  } else if(pg_base == MEM_RET_CROSS_PAGE){
    //printf("before %d %x\n",len,addr);
    return vaddr_read_cross_page(addr,type,len);
  } else {
    printf("Read: pc = 0x%x opcode %x:return MEM_RET_FAIL, Present is 0:pdx = %x ptx = %x vaddr = %x\n",cpu.pc,vaddr_read(cpu.pc,1),0x3ff&(addr>>22),0x3ff&(addr>>12),addr);
    assert(0);
  }
}

void vaddr_write_cross_page(vaddr_t vaddr ,word_t data,int len)
{
  uint8_t a[4] = {data&0xff,(data>>8)&0xff,(data>>16)&0xff,(data>>24)&0xff};
  for(int i = 0;i < len;i ++)
  {
    vaddr_write(page_table_walk(vaddr+i),a[i],1);
    printf("%x",a[i]);
  }
  printf("\n");
}

void vaddr_mmu_write(vaddr_t addr, word_t data, int len){
/*
 *    paddr_t pg_base = isa_mmu_translate(addr, MEM_TYPE_WRITE, len);
 *
 *    if (pg_base == MEM_RET_OK) {
 */
        paddr_t paddr = page_table_walk(addr);
        assert(paddr == addr);
        /*
        word_t ret = data;
        if(len == 4&& (ret&0x80000) == 0x80000 &&
        (ret&0xbfff0000)!=0xbfff0000 && addr == cpu.esp) printf("write addr
        %x data %x pc %x\n",addr,ret,cpu.pc); */
        paddr_write(paddr, data, len);
    /*
     *} else if (pg_base == MEM_RET_CROSS_PAGE) {
     *    // assert(0);
     *    printf("data cross write = %x len %d addr %x\n", data, len, addr);
     *    vaddr_write_cross_page(addr, data, len);
     *    assert(0);
     *} else {
     *    printf("Write pc = 0x%x opcode %x:return MEM_RET_FAIL, Present is "
     *           "0:pdx = %x ptx = %x vaddr = %x\n",
     *           cpu.pc, vaddr_read(cpu.pc, 1), 0x3ff & (addr >> 22),
     *           0x3ff & (addr >> 12), addr);
     *    printf("%x esp %x next %x\n", vaddr_read(cpu.pc + 1, 4), cpu.esp,
     *           vaddr_read(cpu.pc + 5, 4));
     *    assert(0);  // ioe_init crash     check ioe map  read cross may be
     *                // wrong
     *}
     */
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
