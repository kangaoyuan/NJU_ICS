#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char* format, ...) {
    static char buf[256] = {};
    va_list     ap;
    va_start(ap, format);
    int len = vsnprintf(buf, 256, format, ap);
    va_end(ap);
    term->write(buf, len);
}

static void sh_banner() {
    sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
    sh_printf("sh> ");
}

static void sh_handle_cmd(const char* cmd) {
    //static char exec[128] = {0};
    char* exec = (char*)malloc(128);

    if(!cmd)
        return;

    if(!strncmp(cmd, "echo", 4)){
        sh_printf("%s", cmd+5);
    } else {
        memset(exec, 0, 128);
        strcpy(exec, cmd);
        exec[strlen(exec)-1] = '\0';

        int argc = 0;
        char* argv[16] = {0}, *token = strtok(exec, " ");
        while(token){
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }

        argv[argc] = NULL;

        setenv("PATH", "/bin:/usr/bin", 0);
        execvp(argv[0], argv);
    }
}

void builtin_sh_run() {
    sh_banner();
    sh_prompt();

    while (1) {
        SDL_Event ev;
        if (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
                const char* res = term->keypress(handle_key(&ev));
                if (res) {
                    sh_handle_cmd(res);
                    sh_prompt();
                }
            }
        }
        refresh_terminal();
    }
}
