#include <proc.h>

/*
 *typedef struct{
 *    void*     cr3;
 *    uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
 *    uintptr_t irq;
 *    uintptr_t eip, cs, eflags;
 *} Context;
 */
/*
 *typedef struct {
 *    int   pgsize;
 *    Area  area;
 *    void* ptr;
 *} AddrSpace;
 */
/*
 *#define STACK_SIZE (8 * PGSIZE)
 *
 *typedef union {
 *  uint8_t stack[STACK_SIZE] PG_ALIGN;
 *  struct {
 *    Context *cp;
 *    AddrSpace as;
 *    // we do not free memory, so use `max_brk' to determine when to call _map()
 *    uintptr_t max_brk;
 *  };
 *} PCB;
 */

#define MAX_NR_PROC 4

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char* file_name, char* const argv[], char* const envp[]);

PCB *current = NULL;
static PCB pcb_boot = {};
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};

void switch_boot_pcb() {
    current = &pcb_boot;
}

void hello_fun(void* arg){
    int j = 1;
    while (1) {
        Log("Hello World from Nanos-lite with arg '%p' for the %dth time!",
            (uintptr_t)arg, j);
        j++;
        yield();
    }
}

void init_proc() {
    Log("Initializing processes...");
    printf("pcb_boot == %x\n", &pcb_boot);
    printf("pcb[0] == %x\n", &pcb[0]);
    printf("pcb[1] == %x\n", &pcb[1]);
    // load program here

    /*const char file_name[] = "/bin/exectuable_file";*/
    // Attention here, if you do after 4.1, below function maybe invalid.
    //naive_uload(NULL, "/bin/pal");
    
    //context_kload(&pcb[0], hello_fun, NULL);
    context_kload(&pcb[0], hello_fun, (void*)1);
    //context_kload(&pcb[1], hello_fun, (void*)2);
    
    char * const argv[] = {"/bin/pal", "--skip", NULL};
    char * const envp[] = {NULL};
    context_uload(&pcb[1], "/bin/pal", argv, envp);

    //char * const argv[] = {"/bin/menu", NULL};
    //char * const argv[] = {"/bin/nterm", NULL};
    //char * const argv[] = {"/bin/dummy", NULL};
    //char * const argv[] = {"/bin/exec-test", NULL};
    //char * const empty[] = {NULL};
    //context_uload(&pcb[1], "/bin/pal", NULL, NULL);
    //context_uload(&pcb[1], "/bin/menu", argv, empty);
    //context_uload(&pcb[1], "/bin/nterm", argv, empty);
    //context_uload(&pcb[1], "/bin/dummy", argv, empty);
    //context_uload(&pcb[1], "/bin/exec-test", argv, empty);
    switch_boot_pcb();
}

static int sche_cnt = 0, size = 0, choose = -1;
Context* schedule(Context *prev) {
    current->cp = prev;

    //current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);

    if(sche_cnt % 0x2 == 0){
        size++;
        choose = 0;
        sche_cnt = 1;
        current = &pcb[0]; 
        printf("In schedule to pcb[0], current->cp.cr3 == %x\n", current->cp->cr3);
        printf("In schedule to pcb[0], current->cp.eip == %x\n", current->cp->eip);
        printf("In schedule to pcb[0], current->cp.esp == %x\n", current->cp->esp);
        printf("In schedule to pcb[0], current->cp.eax == %x\n", current->cp->eax);
    } else {
        choose = 1;
        sche_cnt++;
        current = &pcb[1]; 
        printf("In schedule to pcb[1], current->cp.cr3 == %x\n", current->cp->cr3);
        printf("In schedule to pcb[1], current->cp.eip == %x\n", current->cp->eip);
        printf("In schedule to pcb[1], current->cp.esp == %x\n", current->cp->esp);
        printf("In schedule to pcb[1], current->cp.eax == %x\n", current->cp->eax);
    }

    printf("shedule: cnt == %x, pcb[%d]\n", sche_cnt, choose);
    if(size == 3)
        assert(0);

    return current->cp;
}
