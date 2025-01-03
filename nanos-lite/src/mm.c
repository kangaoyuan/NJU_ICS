#include <proc.h>
#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
    void* rc = pf;
    // Here we use some compiler-specific for void*
    pf += nr_page * PGSIZE;
    return rc;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
    int cnt = (n + PGSIZE - 1) / PGSIZE;
    void* rc = new_page(cnt);
    memset(rc, 0, cnt*PGSIZE);
    assert(!((uintptr_t)rc % PGSIZE));
    return rc;
}
#endif

void free_page(void *p) {
    (void)p;
    panic("not implement yet");
}

/* The brk() system call handler. */
// if brk exceed current->max_brk, we allocate new_page()
void map(AddrSpace* as, void* va, void* pa, int prot);
int mm_brk(uintptr_t brk) {
    printf("brk == %x\n", brk);
    if(brk > current->max_brk){
        uintptr_t vaddr = current->max_brk;
        uintptr_t vaddr_beg = vaddr & ~((1 << 12) - 1);
        uintptr_t vaddr_end = vaddr_beg + PGSIZE;
        uint32_t len = (vaddr_end - vaddr) <= (brk - vaddr) ? 
                        vaddr_end - vaddr : brk - vaddr;

        if (len < PGSIZE) {
            vaddr += len;
        } else if (len == PGSIZE) {
            assert((vaddr & ((1 << 12) -1)) == 0);
        } else {
            assert(0);
        }

        while(vaddr < brk) {
            void* paddr = new_page(1);
            vaddr_beg = vaddr & ~((1 << 12) - 1);
            vaddr_end = vaddr_beg + PGSIZE;
            len = (vaddr_end - vaddr) <= (brk - vaddr) ? 
                        vaddr_end - vaddr : brk - vaddr;
            map(&current->as, (void*)vaddr_beg, paddr, 0x7);
            vaddr += len;
        } 

        current->max_brk = brk;
    }
    return 0;
}

void init_mm() {
    pf = (void*)ROUNDUP(heap.start, PGSIZE);
    Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
    vme_init(pg_alloc, free_page);
#endif
}
