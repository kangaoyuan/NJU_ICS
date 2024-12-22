#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

extern char **environ;
int main(int argc, char *argv[], char *envp[]);

// Attention to code portably and compatibly for different ISAs.
void call_main(uintptr_t* args) {
    printf("args == %p\n", args);
    int argc = *(int*)args;
    printf("argc == %d\n", argc);
    printf("Hello, Inside call_main\n");
    uintptr_t *argv = (uintptr_t*)((uint8_t*)args + sizeof(int));
    printf("argv == %p\n", argv);
    printf("argv + 1 == %p\n", argv + 1); 
    for(int i = 0; i < argc; ++i){
        printf("argv[%d] == %lx\n", i, *(argv+i)); 
    }
    for(int i = 0; i < argc; ++i){
        char* var = (char*)argv[i];
        printf("argv[%d], var == %lx\n", i, (uintptr_t)var);
        printf("&argv[%d] == %p\n", i, &argv[i]); 
        printf("argv[%d] == %s\n", i, (char*)argv[i]);
    }
    char **envp = (char**)(argv + argc + 1);
    if (envp) {
        for (int i = 0; envp[i]; i++) {
            printf("envp[%d] == %s\n", i, envp[i]);
        }
    }
    environ = envp;
    exit(main(argc, (char**)argv, envp));
    /*
     *char* empty[] = {NULL};
     *environ = empty;
     *exit(main(0, empty, empty));
     */
    assert(0);
}
