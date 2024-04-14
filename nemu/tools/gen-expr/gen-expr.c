#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 1024] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"#include <signal.h>\n"
"void div_handler(int singal) {"
"  printf(\"error\");"
"  exit(-1);"
"}"
"int main() { "
"  signal(SIGFPE, div_handler);"
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static inline int choose(int num){
    return rand() % num;
}

static void gen_char(char a) {
    sprintf(buf+strlen(buf), "%c", a);
    buf[strlen(buf)] = '\0';
}

static inline void gen_rand_num() {
    sprintf(buf+strlen(buf), "%u", (unsigned)rand());
    buf[strlen(buf)] = '\0';
}

static inline void gen_rand_op() {
    const char* op = NULL;
    switch (choose(4)) {
    case 0:
        op = "*";
        break;
    case 1:
        op = "/";
        break;
    case 2:
        op = "+";
        break;
    case 3:
        op = "-";
        break;
    }
    strcat(buf + strlen(buf), op);
    buf[strlen(buf)] = '\0';
 }

 static inline void gen_rand_expr() {
     // Below commenting content is used to generate " " tokens.
     uint32_t space = choose(2);
     for (int i = 0; i < space; ++i)
         gen_char(' ');
     if (strlen(buf) < 7000) {
         switch (choose(3)) {
         case 0:
             gen_rand_num();
             break;
         case 1:
             gen_char('(');
             gen_rand_expr();
             gen_char(')');
             break;
         default:
             gen_rand_expr();
             gen_rand_op();
             gen_rand_expr();
             break;
         }
     } else {
         gen_rand_num();
     }
     buf[strlen(buf)] = '\0';
 }

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int loop = 1;
    if (argc > 1) {
        int ret = sscanf(argv[1], "%d", &loop);
        assert(ret == 1 && loop > 0);
    }

    for (int i = 0; i < loop;) {
        memset(buf, 0, strlen(buf));
        gen_rand_expr();

        sprintf(code_buf, code_format, buf);

        FILE* fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0)
            continue;

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        int result;
        int rv = fscanf(fp, "%d", &result);
        pclose(fp);
        if(rv != 1)
            continue;

        i++;
        printf("result == %u, expr ==  %s\n", result, buf);
    }
    return 0;
}
