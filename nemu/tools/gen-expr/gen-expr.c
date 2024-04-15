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
    unsigned val = rand();
    sprintf(buf+strlen(buf), "%u", val);
    if(val)
        buf[strlen(buf)] = 'U';
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
     uint32_t space = choose(2);
     for (int i = 0; i < space; ++i)
         gen_char(' ');
     if (strlen(buf) < 1000) {
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

        // One way to get rid of /0 error.
        //int ret = system("gcc /tmp/.code.c -Wall -Werror -o /tmp/.expr");
        //if (ret != 0)
        //    continue;

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0)
            continue;

        int result;
        fp = popen("/tmp/.expr", "r");
        if(!fp) {
            perror("Error while opening the process\n");
            return -1;
        }
        int rv = fscanf(fp, "%u", &result);
        if(rv != -1){
            fprintf(stderr, "ignore division\n");
        }

        int status = pclose(fp);
        if (status == -1) {
            perror("Error while closing the process\n");
            return -1;
        } else {
            if(-1 == WEXITSTATUS(status))
               continue;
        }

        for (int i = 0; i < strlen(buf); i++) {
            if (buf[i] == 'U')
                buf[i] = ' ';
        }

        i++;
        printf("result == %u, expr == %s\n", result, buf);
    }
    return 0;
}
