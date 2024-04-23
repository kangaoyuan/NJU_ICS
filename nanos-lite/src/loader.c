#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void* buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
    //TODO();

    // Get Ehdr.
    Elf_Ehdr elf_header;
    int rv = ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));
    assert(rv == sizeof(Elf_Ehdr));

    // Get Phdr.
    Elf_Phdr pro_header[elf_header.e_phnum];
    rv = ramdisk_read(pro_header, elf_header.e_phoff, sizeof(Elf_Phdr)*elf_header.e_phnum);
    assert(rv == sizeof(Elf_Phdr)*elf_header.e_phnum);

    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf_Phdr p = pro_header[i];
        if (p.p_type == PT_LOAD) {
            // for Mem, from p_vaddr to p_vaddr + p_memsz.
            rv = ramdisk_read((void*)p.p_vaddr, p.p_offset, p.p_memsz);
            assert(rv == p.p_memsz);
            // for ELF file, from p_offset to p_offset + p_filesz.
            memset((void*)(p.p_vaddr + p.p_filesz), 0,
                   p.p_memsz - p.p_filesz);
        }
    }

    return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)())entry)();
}

