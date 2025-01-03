#include <proc.h>
#include <elf.h>
#include <fs.h>

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

#define EM_386		 3	/* Intel 80386 */
#define EM_X86_64	62	/* AMD x86-64 architecture */

#if defined(__ISA_AM_NATIVE__) || defined(__ISA_X86_64__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_386
#else
# error Unsupported ISA
#endif

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

int    fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int    fs_close(int fd);
size_t ramdisk_read(void* buf, size_t offset, size_t len);

void show_param();

static uintptr_t loader(PCB *pcb, const char *filename) {
    //TODO();
    int fd = fs_open(filename, 0, 0);

    // Get Ehdr.
    Elf_Ehdr elf_header;
    //int rv = ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));
    int rv = fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
    assert(rv == sizeof(Elf_Ehdr));
    assert(elf_header.e_machine == EXPECT_TYPE);
    uint64_t magic_number = *(uint64_t *)elf_header.e_ident; // Combine first 8 bytes of e_ident
#if defined(__ISA_X86__)
    assert(magic_number == 0x00010101464C457F); // Compare with the expected magic number
#elif defined(__ISA_X86_64__) || (__ISA_AM_NATIVE__)
    assert(magic_number == 0x00010102464C457F); // Compare with the expected magic number
#endif

    // Get Phdr.
    Elf_Phdr pro_header[elf_header.e_phnum];
    //rv = ramdisk_read(pro_header, elf_header.e_phoff, sizeof(Elf_Phdr)*elf_header.e_phnum);
    rv = fs_read(fd, pro_header, sizeof(Elf_Phdr)*elf_header.e_phnum);
    assert(rv == sizeof(Elf_Phdr)*elf_header.e_phnum);

    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf_Phdr p = pro_header[i];

        if (p.p_type == PT_LOAD) {
            fs_lseek(fd, p.p_offset, SEEK_SET);
#ifdef HAS_VME
            // Below content should apply with config to navy
            void* vaddr = (void*)(p.p_vaddr), *paddr = 0;
            void* file_vaddr = (void*)(p.p_vaddr + p.p_filesz);
            void* mem_vaddr = (void*)(p.p_vaddr + p.p_memsz);
            
            while(vaddr < file_vaddr){
                paddr = new_page(1);
                void* vaddr_beg = (void*)((uintptr_t)vaddr & ~(0xfff));
                void* vaddr_end = (void*)((uintptr_t)vaddr & ~(0xfff)) + PGSIZE;
                map(&pcb->as, vaddr_beg, paddr, 0x7);
                uint32_t vaddr_off = (uintptr_t)vaddr & (0xfff);
                uint32_t read_cnt = (vaddr_end  - vaddr) <= (file_vaddr - vaddr) ?
                (vaddr_end  - vaddr) : (file_vaddr - vaddr);
                fs_read(fd, paddr + vaddr_off, read_cnt);
                vaddr += read_cnt;
            }
            assert(vaddr == file_vaddr);

            while(vaddr < mem_vaddr){
                uint32_t vaddr_off = (uintptr_t)vaddr & (0xfff);
                void* vaddr_beg = (void*)((uintptr_t)vaddr & ~(0xfff));
                void* vaddr_end = (void*)((uintptr_t)vaddr & ~(0xfff)) + PGSIZE;
                if(vaddr_off == 0){
                    paddr = new_page(1); 
                    map(&pcb->as, vaddr_beg, paddr, 0x7);
                }
                uint32_t read_cnt = (vaddr_end  - vaddr) <= (mem_vaddr - vaddr) ?
                (vaddr_end  - vaddr) : (mem_vaddr - vaddr);
                memset(paddr + vaddr_off, 0, read_cnt);
                vaddr += read_cnt;
            }
            assert(vaddr == mem_vaddr);
            pcb->max_brk = (uintptr_t)vaddr;
#else
            (void)pcb;
            // for Mem, from p_vaddr to p_vaddr + p_memsz.
            //rv = ramdisk_read((void*)p.p_vaddr, p.p_offset, p.p_memsz);
            rv = fs_read(fd, (void *)p.p_vaddr, p.p_filesz);
            assert((size_t)rv == p.p_filesz);
            // for ELF file, from p_offset to p_offset + p_filesz.
            memset((void*)(p.p_vaddr + p.p_filesz), 0,
                   p.p_memsz - p.p_filesz);
#endif
        }
    }

    fs_close(fd);
    return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *file_name) {
    uintptr_t entry = loader(pcb, file_name);
    Log("Jump to entry = %p", entry);
    ((void (*)())entry)();
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg){
    Area kstack;
    kstack.start = pcb->stack;
    kstack.end = kstack.start + sizeof(pcb->stack);
    pcb->cp = kcontext(kstack, entry, arg);
}

void* create_stack(void* stack_top, char* const* argv, char* const envp[]) {
    uint32_t argc = 0, envc = 0, size_argv = 0, size_envp = 0;

    if (argv) {
        while (argv[argc]) {
            size_argv += strlen(argv[argc++]) + 1;
        }
    }

    if (envp) {
        while (envp[envc]) {
            size_envp += strlen(envp[envc++]) + 1;
        }
    }

    uint32_t size = sizeof(uint32_t) +
                    (argc + envc + 4) * sizeof(uintptr_t) + size_argv +
                    size_envp;
    size = size - size % (2 * sizeof(uintptr_t));

    void* argc_start = stack_top - size;
    void* str_start = argc_start + sizeof(uint32_t) +
                      (argc + envc + 2) * sizeof(uintptr_t);

    memset(argc_start, 0, size);
    *(uint32_t*)argc_start = argc;

    uintptr_t* argv_start =
        (uintptr_t*)((uint8_t*)argc_start + sizeof(uint32_t));
    for (uint32_t i = 0; i < argc; ++i) {
        argv_start[i] = (uintptr_t)str_start;
        memcpy(str_start, argv[i], strlen(argv[i]));
        str_start += strlen(argv[i]) + 1;
    }
    argv_start[argc] = (uintptr_t)NULL;

    uintptr_t* envp_start =
        (uintptr_t*)((uint8_t*)argc_start + sizeof(uint32_t) +
                     (argc + 1) * sizeof(uintptr_t));
    for (uint32_t i = 0; i < envc; ++i) {
        envp_start[i] = (uintptr_t)str_start;
        memcpy(str_start, envp[i], strlen(envp[i]));
        str_start += strlen(envp[i]) + 1;
    }
    envp_start[envc] = (uintptr_t)NULL;

    return argc_start;
}

void context_uload(PCB *pcb, const char *file_name, char* const argv[], char* const envp[]){
    // Copy kernel mapping
    protect(&pcb->as);

    // Below sequence for creating stack and loader
    // is so important for the overwritting problem.
    void* user_stack = new_page(8) + 8 * PGSIZE;
    printf("Inside context_uload, user_stack == %x\n", user_stack);
    for(int i = 8; i > 0; --i){
        printf("stack: CR3 == %x\n", pcb->as.ptr);
        printf("stack: %x -> %x\n", pcb->as.area.end - i * PGSIZE, user_stack - i * PGSIZE);
        map(&pcb->as, pcb->as.area.end - i * PGSIZE, user_stack - i * PGSIZE, 0x7);
    }
    void* stack_ptr = create_stack(user_stack, argv, envp);

    /*
     *void* stack_ptr = create_stack(heap.end, argv, envp);
     */
    /*
     *void* user_stack = new_page(8) + 8 * PGSIZE;
     *void* stack_ptr = create_stack(user_stack, argv, envp);
     */

    Area kstack = {.start = pcb->stack,
                   .end = pcb->stack + sizeof(pcb->stack)};
    printf("Before loader\n");
    void *entry = (void*)loader(pcb, file_name);
    printf("Before ucontext\n");
    pcb->cp = ucontext(&pcb->as, kstack, entry);
    //pcb->cp->GPRx = (uintptr_t)stack_ptr;
    // For the actual memory mapping.
    pcb->cp->GPRx = (uintptr_t)(pcb->as.area.end - (user_stack - stack_ptr));
    pcb->cp->esp3 = (uintptr_t)(pcb->as.area.end - (user_stack - stack_ptr));
    printf("stack start at: %x\n", pcb->cp->GPRx);
}
