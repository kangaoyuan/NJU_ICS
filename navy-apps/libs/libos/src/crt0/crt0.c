#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

extern char **environ;
int main(int argc, char *argv[], char *envp[]);

// Attention to code portably and compatibly for different ISAs.
void call_main(uintptr_t* args) {
    int argc = *(int*)args;
    char **argv = (char**)((uint8_t*)args + sizeof(int));
    char **envp = (char**)(argv + argc + 1);
    environ = envp;
    exit(main(argc, argv, envp));
    assert(0);
}
