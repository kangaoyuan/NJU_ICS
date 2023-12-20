#include <common.h>
#include <stdlib.h>
#include <readline/readline.h>

void init_monitor(int, char *[]);
void engine_start();
int is_exit_status_bad();

char* rl_gets(); /* {
    static char* line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(expr) ");

    return line_read;
} */

word_t expr(char *e, bool *success);
void test_expr() {
    for (char *str; (str = rl_gets()) != NULL; ) {
        char *expression;
        uint32_t real = strtol(str,&expression,10);

        bool success;
        uint32_t actual = expr(expression,&success);
        if (success) {
            if (real != actual) {
                printf("Wrong: %s, real is %u, actual is %u\n", expression, real, actual);
                assert(0);
            }
            else
                printf("Accept!\n");
        }
        else
            assert(0);
    }
}

int main(int argc, char* argv[]) {
    /* Initialize the monitor. */
    init_monitor(argc, argv);

    /* Test expression eval.*/
    test_expr();

    /* Start engine. */
    //engine_start();

    return is_exit_status_bad();
}
