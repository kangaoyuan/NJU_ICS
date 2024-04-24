#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
size_t ramdisk_read(void* buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
    //TODO();
    int fd = fs_open(filename, 0, 0);
    assert(fd >= 0);

    // Get Ehdr.
    Elf_Ehdr elf_header;
    /*int rv = ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));*/
    int rv = fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
    assert(rv == sizeof(Elf_Ehdr));

    // Get Phdr.
    Elf_Phdr pro_header[elf_header.e_phnum];
    /*rv = ramdisk_read(pro_header, elf_header.e_phoff, sizeof(Elf_Phdr)*elf_header.e_phnum);*/
    rv = fs_read(fd, pro_header, sizeof(Elf_Phdr)*elf_header.e_phnum);
    assert(rv == sizeof(Elf_Phdr)*elf_header.e_phnum);

    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf_Phdr p = pro_header[i];
        if (p.p_type == PT_LOAD) {
            // for Mem, from p_vaddr to p_vaddr + p_memsz.
            fs_lseek(fd, p.p_offset, 0);
            rv = fs_read(fd, (void *)p.p_vaddr, p.p_memsz);
            /*rv = ramdisk_read((void*)p.p_vaddr, p.p_offset, p.p_memsz);*/
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
    printf("Can I reach the end of naive_uload\n");
}

