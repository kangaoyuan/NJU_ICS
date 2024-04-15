#include <debug.h>
#include <common.h>
#include "monitor/debug/expr.h"

void engine_start();
int is_exit_status_bad();
void init_monitor(int, char *[]);

void test_expr(){
    FILE* fp = fopen("tools/gen-expr/input", "r");
    assert(fp!=NULL);

    // Here loop is hard code, corresponding to the arg of gen-expr tool.
    for (int i = 0; i < 700; i++) {
        bool     flag;
        unsigned ans, eval;
        char     expression[70000] = {0};
        int rv = fscanf(fp, "result == %u, expr == %[^\n] ", &ans, expression);
        Assert(rv == 2, "test_expr failed");
        eval = expr(expression, &flag);
        Assert(flag == true, "test_expr failed");
        if (ans != eval){
            printf("Wrong %d:\n", i);
            printf("result == %u, eval == %u\n", ans, eval);
        }
        else
            printf("\33[1;31m""correct %d""\33[0m\n", i);
    }
    fclose(fp);
}

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
