#include <common.h>
#include <readline/readline.h>

void init_monitor(int, char *[]);
void test_expr(); 
void engine_start();
int is_exit_status_bad();


int main(int argc, char* argv[]) {
    /* Initialize the monitor. */
    init_monitor(argc, argv);

    /* Test expression eval.*/
#if defined(EXPR_TEST)
    test_expr();
#else
    /* Start engine. */
    engine_start();
#endif

    return is_exit_status_bad();
}
