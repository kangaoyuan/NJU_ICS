#ifndef __MEMORY_PADDR_H__
#define __MEMORY_PADDR_H__

#include <common.h>

#define PMEM_SIZE (128 * 1024 * 1024)

/* convert the guest physical address in the guest program to host virtual address in NEMU */
uint8_t* guest_to_host(paddr_t addr);
/* convert the host virtual address in NEMU to guest physical address in the guest program */
paddr_t  host_to_guest(uint8_t *addr);

word_t paddr_read(paddr_t addr, int len);
void   paddr_write(paddr_t addr, word_t data, int len);

#endif
