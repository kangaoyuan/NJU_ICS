#include <debug.h>
#include <common.h>
#include "monitor/debug/expr.h"

void engine_start();
int is_exit_status_bad();
void init_monitor(int, char *[]);

void test_expr(){
    FILE* fp = fopen("tools/gen-expr/input", "r");
    assert(fp!=NULL);

    char  expression[70000];
    for (int i = 0; i < 10000; i++) {
        bool flag = true;
        unsigned ans = 0, eval = 0;
        int rv = fscanf(fp, "result == %u, expr == %[^\n]", &ans, expression);
        Assert(rv == 2, "test_expr failed");
        eval = expr(expression, &flag);
        Assert(flag == true, "test_expr failed");
        if (ans != eval){
            printf("Wrong %d:\n", i);
            printf("result == %u, eval == %u\n", ans, eval);
        }
        else
            printf("correct %d\n", i);
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {
    /* Initialize the monitor. */
    init_monitor(argc, argv);

    test_expr();

    /* Start engine. */
    engine_start();

    return is_exit_status_bad();
}
