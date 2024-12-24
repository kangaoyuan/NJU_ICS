#include <am.h>
#include <nemu.h>
#include <klib.h>

static int vme_enable = 0;
static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;

static Area segments[] = {      // Kernel memory mappings with three parts
    NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
    pgalloc_usr = pgalloc_f;
    pgfree_usr = pgfree_f;

    kas.ptr = pgalloc_f(PGSIZE);

    for (uint32_t i = 0; i < LENGTH(segments); i++) {
        for (void* va = segments[i].start; va < segments[i].end; va += PGSIZE) {
            map(&kas, va, va, 0);
        }
    }
    printf("Inside vme_init, cpu.CR3 == %x\n", kas.ptr);

    set_cr3(kas.ptr);
    printf("Inside vme_init, cpu.CR3 == %x\n", kas.ptr);
    set_cr0(get_cr0() | CR0_PG);
    printf("Inside vme_init, cpu.CR3 == %x\n", kas.ptr);
    vme_enable = 1;

    return true;
}

void protect(AddrSpace* as) {
    PTE* updir = (PTE*)(pgalloc_usr(PGSIZE));
    as->ptr = updir;
    as->area = USER_SPACE;
    as->pgsize = PGSIZE;
    // map kernel space
    memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
    (void)as;
}

void __am_get_cur_as(Context *c) {
    c->cr3 = (vme_enable ? (void*)get_cr3() : NULL);
}

void __am_switch(Context *c) {
    if (vme_enable && c->cr3 != NULL) {
        set_cr3(c->cr3);
    }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
    (void)prot;
    assert(as && as->ptr);

    uint32_t* pg_dir = as->ptr;
    uint32_t va_dir = (uint32_t)va >> 22;
    uint32_t va_pg = (uint32_t)va >> 12 & ((1 << 10) - 1);
    uint32_t pa_pg = (uint32_t)pa >> 12;
    uint32_t* pg_tbl = &pg_dir[va_dir];

    if((*pg_tbl & 1) == 0) {
        *pg_tbl = (uint32_t)pgalloc_usr(PGSIZE) | 1;
    }

    uint32_t* pg_pte = (uint32_t*)(*pg_tbl & ~((1 << 12) - 1)); 
    if((pg_pte[va_pg] & 1) == 0) {
        pg_pte[va_pg] = pa_pg << 12 | 1;
    }
}

// Create a context with a independent address space.
// The kstack is the same as the kernel thread to allocate Context.
Context* ucontext(AddrSpace *as, Area kstack, void *entry) {
    (void)as;
    Context* ucontext = (Context*)kstack.end - 1;
    //Context* ucontext = kstack.end - sizeof(Context);
    *ucontext = (Context){.cr3 = NULL,
                 .eip = (uintptr_t)entry,
                 .cs  = 0x8,
                 .eflags = 0x200};
    return ucontext;
}
