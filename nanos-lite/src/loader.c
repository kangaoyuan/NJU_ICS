#include <elf.h>
#include <proc.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB* pcb, const char* filename) {
    TODO();
    //assert(*(uint32_t *)elf->e_ident == 0xBadC0de);
    return 0;
}

void naive_uload(PCB* pcb, const char* filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)())entry)();
}
