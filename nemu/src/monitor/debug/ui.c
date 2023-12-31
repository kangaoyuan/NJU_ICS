#include <isa.h>
#include "expr.h"
#include "watchpoint.h"
#include "memory/vaddr.h"
#include "monitor/monitor.h"
#include <stdlib.h>
#include <readline/history.h>
#include <readline/readline.h>

int is_batch_mode();
void cpu_exec(uint64_t);

/* We use the `readline' library to read from stdin. */
char* rl_gets(const char* prompt) {
    static char* line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline(prompt);

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char* args) {
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char* args) {
    nemu_state.state = NEMU_QUIT;
    return -1;
}

static int cmd_si(char* args) {
    if (args == NULL){
        cpu_exec(1);
        return 0;
    }
    int n = -1;
    if (sscanf(args, "%d", &n) == 1 && n > 0) {
        cpu_exec(n);
    } else {
        printf("Invalid arg, (nemu) si command args error: \e[0;31m%s\e[0m\n", args);
    }
    return 0;
}

static int cmd_info(char* args) {
    if (!strcmp(args, "r")) {
        isa_reg_display();
    } else if (!strcmp(args, "w")) {
        wp_display();
    } else {
        printf("Invalid arg, (nemu) info command args error: \e[0;31m%s\e[0m\n", args); 
    }
    return 0;
}

static int cmd_x(char* args) {
    if (args == NULL) {
        printf("(nemu) x command missing SUBCMD\n");
        return 0;
    }

    int64_t n = -1;
    char* token = strtok(args, " ");
    int rc = sscanf(token, "%ld", &n);
    if (rc != 1 || n < 0) {
        printf("Invalid arg, (nemu) x command args error: \e[0;31m%s\e[0m\n", args);
        return 0;
    }

    // In each subsequent call that should parse the same string, str must be NULL.
    token = strtok(NULL, "");
    // If no more tokens are found, strtok() returns NULL.
    if (token == NULL) {
        printf("(nemu) x command missing SUBCMD\n");
        return 0;
    }

    bool success;
    uint32_t addr_val = expr(token, &success);
    assert(success == true);
    for (int i = 0; i < n; i++) {
        word_t content = vaddr_read(addr_val + i * 4, 4);
        printf("addr:0x%08x%*sval:0x%08x%*s%d\n", addr_val + i * 4, 4, "",
               content, 4, "", content);
    }
    return 0;
}

static int cmd_p(char* args) {
    if (args == NULL) {
        printf("missing p SUBCMD\n");
        return 0;
    }
    bool success;
    uint32_t val = expr(args, &success);
    assert(success == true);
    printf("%s:\t0x%08x\t%d\n", args, val, val);
    return 0;
}

static int cmd_w(char* args) {
    new_wp(args);
    return 0;
}

static int cmd_d(char* args) {
    int no;
    sscanf(args, "%d", &no);
    free_wp(no);
    return 0;
}

static int cmd_help(char *args);

static struct {
    char* name;
    char* description;
    int (*handler)(char*);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "si [N] - Let program excute N steps, default N is 1", cmd_si},
    {"info",
     "info [rw] r - print the status of regs, w - print watching point "
     "info",
     cmd_info},
    {"x",
     "x [N] [EXPR], evaluate the EXPR, use the result as the start address "
     "and output N consecutive 4 bytes in hex form",
     cmd_x},
    {"p", "evaluate", cmd_p},
    {"w", "w [EXPR] watchpoint, stop the program when EXPR is true", cmd_w},
    {"d", "d [N] delete the watchpoint", cmd_d},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char* args) {
    /* extract the first argument */
    char* arg = strtok(NULL, " ");
    int   i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name,
                   cmd_table[i].description);
        }
    } else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name,
                       cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void ui_mainloop() {
    if (is_batch_mode()) {
        cmd_c(NULL);
        return;
    }

    for (char* str; (str = rl_gets("(nemu) ")) != NULL;) {
        char* str_end = str + strlen(str);

        /* extract the first token as the command. */
        char* cmd = strtok(str, " ");
        if (cmd == NULL) {
            continue;
        }

        /* treat the remaining string as the arguments,
         * which may need further parsing. */
        char* args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

#ifdef HAS_IOE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                if (cmd_table[i].handler(args) < 0) {
                    return;
                }
                break;
            }
        }

        // (nemu) prompt for debug purpose's command is not supported.
        if (i == NR_CMD) {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}
