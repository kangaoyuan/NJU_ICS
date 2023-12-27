#include <elf.h>
#include <proc.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t get_ramdisk_size();
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB* pcb, const char* filename) {
    Elf_Ehdr elf_header;
    size_t offset = ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));
    // offset(return val) is equal to ramdisk_read() third arg.
    assert(offset == sizeof(Elf_Ehdr));

    // Attention: . -> () [] have higher precedence than () for casting.
    //assert(*(uint32_t*)elf_header->e_ident == 0x464c457f);
    Elf_Phdr elf_program_header[elf_header.e_phnum];
    offset = ramdisk_read(elf_program_header, elf_header.e_phoff,
                          sizeof(Elf_Phdr) * elf_header.e_phnum);
    // offset(return val) is equal to ramdisk_read() third arg.
    assert(offset == sizeof(Elf_Phdr) * elf_header.e_phnum);

    for (int i = 0; i < elf_header.e_phnum; i++) {
        // only load PT_LOAD type
        if (elf_program_header[i].p_type == PT_LOAD) {
            ramdisk_read((void*)elf_program_header[i].p_vaddr,
                         elf_program_header[i].p_offset,
                         elf_program_header[i].p_memsz);
            // clear the [Virtual Address + File Size, Virtual Address +
            // Memory Size)
            memset((void*)(elf_program_header[i].p_vaddr +
                           elf_program_header[i].p_filesz),
                   0,
                   elf_program_header[i].p_memsz -
                   elf_program_header[i].p_filesz);
        }
    }
    return elf_header.e_entry;
}

void naive_uload(PCB* pcb, const char* filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)())entry)();
}
