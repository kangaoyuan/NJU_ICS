#include <isa.h>
#include "expr.h"
#include "watchpoint.h"
#include <memory/vaddr.h>
#include <monitor/monitor.h>

#include <stdlib.h>
#include <readline/history.h>
#include <readline/readline.h>

int is_batch_mode();
void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
    static char* line_read = NULL;

    if (line_read) {
        // malloc() is called in readline() and returns the text of the line read, which has the final newline removed.
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

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

static int cmd_info(char* args){
    if(!strcmp(args, "r")){
        isa_reg_display();
        return 0;
    }else if(!strcmp(args, "w")){
        wp_pool_display();
        return 0;
    } else {
        printf("Invalid arg, (nemu) info command args error: "
               "\e[0;31m%s\e[0m\n",
               args);
    }

    return -1;
}

static int cmd_x(char* args){
    if(!args){
        printf("missing x <num> <expr> args\n");
        return 0;
    }

    char* args_num = strtok(args, " ");
    char* args_expr = strtok(NULL, "");
    
    printf("In cmd_x args_num == %s\n", args_num);
    printf("In cmd_x args_num == %s\n", args_expr);

    int num = strtol(args_num, NULL, 10);
    bool flag;
    int val = expr(args_expr, &flag);

    if (!flag) {
        printf("sdb cmd: x %s %s, Wrong expression\n", args_num, args_expr);
        return -1;
    }

    for (int i = 0; i < num; i++) {
        printf("addr:0x%08x %08x\n", val + 4*i, vaddr_read(val + 4*i, 4));
    }
    return 0;
}

static int cmd_p(char* args){
    if (args == NULL) {
        printf("missing p <expr> args\n");
        return 0;
    }
    bool flag;
    uint32_t val = expr(args, &flag);
    if (!flag) {
        printf("sdb cmd: p %s, Wrong expression\n", args);
        return -1;
    }
    printf("%s:\t%08d\t0x%08x\n", args, val, val);
    return 0;
}

static int cmd_w(char* args){
    bool flag;
    uint32_t val = expr(args, &flag);
    if(!flag){
        printf("Invalid arg, (nemu) w <expr> command args error: \e[0;31m%s\e[0m\n", args);
    }

    WP* cur = new_wp();
    strcpy(cur->expr, args);
    cur->pre_val = 0;
    cur->cur_val = val;
    printf("Watchpoint %d:%s\n", cur->NO, cur->expr);
    return 0;
}

static int cmd_d(char* args){
    uint32_t n;
    int rc = sscanf(args, "%d", &n);
    if (rc != 1 || n < 0) {
        printf("Invalid arg, (nemu) d <Num> command args error: \e[0;31m%s\e[0m\n", args);
        return 0;
    }

    free_wp(n);
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
    {"si", "Step [N] instruction, default N is 1", cmd_si},
    {"info", "Info <r|w>", cmd_info},
    {"x", "Examine <Num> <address> for hex format with int length", cmd_x},
    {"p", "Print <expr>", cmd_p},
    {"w", "Watchpoint <expr>", cmd_w},
    {"d", "Delete <Num> watchpoint", cmd_d},

    /* TODO: Add more commands */

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

    for (char* str; (str = rl_gets()) != NULL;) {
        char* str_end = str + strlen(str);

        /* extract the first token as the command */
        char* cmd = strtok(str, " ");
        if (cmd == NULL) {
            continue;
        }

        /* treat the remaining string as the arguments,
         * which may need further parsing
         */
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

        if (i == NR_CMD) {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}
