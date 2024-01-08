#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

// this should be enough
uint32_t buf_index = 0;
static char  buf[65536] = {};
static char  code_buf[65536 + 128] = {};  // a little larger than `buf`
static char* code_format = "#include <stdio.h>\n"
                           "int main() { "
                           "  unsigned result = %s; "
                           "  printf(\"%%u\", result); "
                           "  return 0; "
                           "}";

uint32_t choose(uint32_t n) {
    return rand() / ((RAND_MAX + 1u) / n);
}

void gen(char c) {
    buf[buf_index++] = c;
    return;
}

void gen_num() {
    uint32_t num = rand();
    if (num == 0)
        num++;

    sprintf(&buf[buf_index], "%u", num);
    //sprintf(buf + buf_index, "%u", num);

    while (num) {
        num /= 10;
        buf_index++;
    }
}

void gen_rand_op() {
    switch (choose(4)) {
    case 0:
        buf[buf_index++] = '+';
        break;
    case 1:
        buf[buf_index++] = '-';
        break;
    case 2:
        buf[buf_index++] = '*';
        break;
    default:
        buf[buf_index++] = '/';
        break;
    }
}

static void gen_rand_expr() {
    uint32_t space = choose(2);
    for (int i = 0; i < space; ++i)
        buf[buf_index++] = ' ';

    switch (choose(3)) {
    case 0: 
        gen_num();
        break;
    case 1:
        if (buf_index < 50) {
            gen('(');
            gen_rand_expr();
            gen(')');
        } else {
            gen('(');
            gen_num();
            gen(')');
        }
        break;
    default:
        if (buf_index < 50) {
            gen_rand_expr();
            gen_rand_op();
            gen_rand_expr();
        } else {
            gen_num();
            gen_rand_op();
            gen_num();
        }
        break;
    }
}

int main(int argc, char* argv[]) {
    int seed = time(NULL);
    srand(seed);

    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }

    for (int i = 0; i < loop; i++) {
        buf_index = 0;
        gen_rand_expr();
        buf[buf_index] = '\0';

        sprintf(code_buf, code_format, buf);

        FILE* fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        // system() using execl("/bin/sh", "sh", "-c", command, (char *)NULL); and returns after the command has benn completed.
        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0)
            continue;

        // Since a pipe is by definition unidirectional, the type argument may specify only reading or writing, not both.
        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        int result;
        // "r" mode to read the output of the command specifeid in popen().
        int rc = fscanf(fp, "%d", &result);
        assert(rc == 1);

        // FILE* returned from popen() must be closed with pclose() not fclose(), which are block buffered by default.
        pclose(fp);

        printf("%u %s\n", result, buf);
    }
    return 0;
}
