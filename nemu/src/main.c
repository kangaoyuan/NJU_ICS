#include <debug.h>
#include <common.h>

void init_monitor(int, char *[]);
void test_expr();
void engine_start();
int is_exit_status_bad();

int main(int argc, char* argv[]) {
    /* Initialize the monitor. */
    init_monitor(argc, argv);

#ifdef TEST_EXPR
    /* Start test. */
    test_expr();
    return 0;
#else
    /* Start engine. */
    engine_start();
    return is_exit_status_bad();
#endif
}
