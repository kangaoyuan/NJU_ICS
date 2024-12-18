#include <proc.h>

#define MAX_NR_PROC 4

void naive_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
    /*switch_boot_pcb();*/

    Log("Initializing processes...");

    // load program here
    /*const char file_name[] = "/bin/exectuable_file";*/
    /*naive_uload(NULL, file_name);*/
    naive_uload(NULL, "/bin/typing-game");
}

Context* schedule(Context *prev) {
  return NULL;
}
