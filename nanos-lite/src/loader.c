#include <proc.h>
#include <elf.h>
#include <fs.h>

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
    (void)pcb;
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
            fs_lseek(fd, p.p_offset, 0);
#ifdef HAV_VME
#else
            // for Mem, from p_vaddr to p_vaddr + p_memsz.
            //rv = ramdisk_read((void*)p.p_vaddr, p.p_offset, p.p_memsz);
            rv = fs_read(fd, (void *)p.p_vaddr, p.p_memsz);
            assert((size_t)rv == p.p_memsz);
            // for ELF file, from p_offset to p_offset + p_filesz.
            memset((void*)(p.p_vaddr + p.p_filesz), 0,
                   p.p_memsz - p.p_filesz);
#endif
        }
    }
    //show_param();

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

void* create_stack(void* stack_top, char * const *argv, char * const envp[]){
    uint32_t argc = 0, envc = 0, size_argv = 0, size_envp = 0;

    if(argv){
        while (argv[argc]) {
            size_argv += strlen(argv[argc++]) + 1;
        }
    }
    

    if(envp){
        while (envp[envc]) {
            size_envp += strlen(envp[envc++]) + 1;
        }
    }

    uint32_t size = sizeof(uint32_t) + (argc + envc + 4) * sizeof(uintptr_t) + size_argv + size_envp;
    size = size - size % (2 * sizeof(uintptr_t));
    
    void* argc_start = stack_top - size;
    void* str_start = argc_start +  sizeof(uint32_t) + (argc + envc + 2) * sizeof(uintptr_t);
    printf("Creat stack, argc_start == %x\n", argc_start);
    printf("Creat stack, str_start == %x\n", str_start);


    memset(argc_start, 0, size);
    *(uint32_t*)argc_start = argc;
    printf("Creat stack, argc_start == %x, %d\n", argc_start, *(uint32_t*)argc_start);

    uintptr_t* argv_start = (uintptr_t*)((uint8_t*)argc_start + sizeof(uint32_t));
    printf("Creat stack, argv_start  == %p\n", argv_start);
    printf("Creat stack, argv_start + 1  == %p\n", argv_start + 1);
    for(uint32_t i = 0; i < argc; ++i){
        argv_start[i] = (uintptr_t)str_start;
        memcpy(str_start, argv[i], strlen(argv[i])); 
        printf("Creat stack, str_start == %s\n", str_start);
        printf("Creat stack, str_start == %s\n", argv_start[i]);
        printf("Creat stack, argv + %d  == %p\n", i, argv_start + i);
        printf("Creat stack, argv[%d]  == %x\n", i, argv_start[i]);
        str_start += strlen(argv[i]) + 1;
    }
    argv_start[argc] = (uintptr_t)NULL;
    printf("Creat stack, argv_start + argc  == %p\n", argv_start + argc);
    printf("Creat stack, argv[%d]  == %x\n", argv_start[argc]);

    uintptr_t* envp_start = (uintptr_t*)((uint8_t*)argc_start + sizeof(uint32_t) + (argc+1) * sizeof(uintptr_t));
    printf("Creat stack, envp_start  == %p\n", envp_start);
    printf("Creat stack, envp_start + 1  == %p\n", envp_start + 1);
    for(uint32_t i = 0; i < envc; ++i){
        envp_start[i] = (uintptr_t)str_start;
        memcpy(str_start, envp[i], strlen(envp[i])); 
        printf("Creat stack, envp + %d == %p\n", i, envp_start + i);
        printf("Creat stack, envp[%d]  == %x\n", i, envp_start[i]);
        str_start += strlen(envp[i]) + 1;
    }
    envp_start[envc] = (uintptr_t)NULL;
    printf("Creat stack, envp_start + envc  == %p\n", envp_start + envc);
    printf("Creat stack, envp[%d]  == %x\n", envp_start[envc]);

    return argc_start;
}

static char* const* argv_;
static char* const* envp_;
void show_param() {
    if (argv_) {
        for (int i = 0; argv_[i]; i++) {
            printf("argv[%d] == %s\n", i, argv_[i]);
        }
    }
    if (envp_) {
        for (int i = 0; envp_[i]; i++) {
            printf("envp[%d] == %s\n", i, envp_[i]);
        }
    }
}
void context_uload(PCB *pcb, const char *file_name, char* const argv[], char* const envp[]){
    argv_ = argv, envp_ = envp;
    printf("Inside context_uload, argv == %x, envp == %x\n", argv, envp);
    printf("file_name == %s\n", file_name);
    if (argv) {
        for (int i = 0; argv[i]; i++) {
            printf("argv[%d] == %s\n", i, argv[i]);
        }
    }
    if (envp) {
        for (int i = 0; envp[i]; i++) {
            printf("envp[%d] == %s\n", i, envp[i]);
        }
    }
    printf("Inside context_uload, Here we can access envp\n");

    AddrSpace* as = &pcb->as;
    Area kstack = {.start = pcb->stack,
                   .end = pcb->stack + sizeof(pcb->stack)};

    void* user_stack = new_page(8);
    printf("user_stack == %x\n", user_stack);
    void* stack_ptr = create_stack(user_stack, argv, envp);

    void *entry = (void*)loader(pcb, file_name);
    printf("Inside context_uload to loader, entry == %p\n", entry);
    pcb->cp = ucontext(as, kstack, entry);
    pcb->cp->GPRx = (uintptr_t)stack_ptr;

    /*
     *void* user_stack = new_page(8);
     *printf("user_stack == %x\n", user_stack);
     *pcb->cp->GPRx = (uintptr_t)create_stack(user_stack, argv, envp);
     */

     //pcb->cp->GPRx = (uintptr_t)create_stack(heap.end, argv, envp);
}
