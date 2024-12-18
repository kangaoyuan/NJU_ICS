#include <proc.h>

/*
 *struct Context {
 *    void*     cr3;
 *    uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
 *    uintptr_t irq;
 *    uintptr_t eip, cs, eflags;
 *};
 */
/*
 *# stack size is 32K
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

PCB *current = NULL;
static PCB pcb_boot = {};
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};

void switch_boot_pcb() {
    current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j++;
    yield();
  }
}

void init_proc() {
    /*switch_boot_pcb();*/

    Log("Initializing processes...");

    // load program here
    /*const char file_name[] = "/bin/exectuable_file";*/
    /*naive_uload(NULL, file_name);*/
    //naive_uload(NULL, "/bin/nterm");
    context_kload(&pcb[0], hello_fun, NULL);
    switch_boot_pcb();
}

Context* schedule(Context *prev) {
    current->cp = prev;
    current = &pcb[0];

    return current->cp;
}
