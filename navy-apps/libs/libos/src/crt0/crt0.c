#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

extern char **environ;
int main(int argc, char *argv[], char *envp[]);

void call_main(uintptr_t* args) {
    char* empty[] = {NULL};
    environ = empty;
    exit(main(0, empty, empty));
    assert(0);
}
