#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
    void* rc = pf;
    pf += nr_page * PGSIZE;
    // There is a little difference to the manual.
    return rc;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
    int cnt = (n + PGSIZE - 1) / PGSIZE;
    void* rc = new_page(cnt);
    memset(rc, 0, n);
    return rc;
}
#endif

void free_page(void *p) {
    (void)p;
    panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk __attribute__((unused))) {
    return 0;
}

void init_mm() {
    pf = (void*)ROUNDUP(heap.start, PGSIZE);
    Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
    vme_init(pg_alloc, free_page);
#endif
}
