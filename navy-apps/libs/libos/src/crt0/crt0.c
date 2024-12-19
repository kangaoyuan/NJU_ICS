#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

extern char **environ;
int main(int argc, char *argv[], char *envp[]);

// Attention to code portably and compatibly for different ISAs.
void call_main(uintptr_t* args) {
    int argc = *(uint32_t*)args;
    char **argv = (char**)((uint8_t*)args + sizeof(uint32_t));
    char **envp = (char**)((uint8_t*)args + (argc + 2) * sizeof(uint32_t));
    environ = envp;
    exit(main(argc, argv, envp));
    assert(0);
}
