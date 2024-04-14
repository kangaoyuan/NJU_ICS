#include <debug.h>
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */

#include <regex.h>
#include "memory/vaddr.h"

enum {

    /* TODO: Add more token types */

    TK_NOTYPE = 256,
    TK_NEG,
    TK_DEREF,
    TK_DEC,
    TK_HEX,
    TK_REG,
    TK_EQ,
    TK_NEQ,
    TK_AND,
    TK_OR,
};

static struct rule {
    char* regex;
    int   token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},              // spaces
    {"\\(", '('},                   // left_parenthesis
    {"\\)", ')'},                   // right_parenthesis
    {"0[xX][0-9a-fA-F]+", TK_HEX},  // hex
    {"[0-9]+", TK_DEC},             // decimal
    {"\\$[a-zA-Z]+", TK_REG},       // registers
    {"\\*", '*'},                   // multiple
    {"/", '/'},                     // division
    {"\\+", '+'},                   // plus
    {"-", '-'},                     // subtract
    {"==", TK_EQ},                  // equal
    {"!=", TK_NEQ},                 // not_equal
    {"&&", TK_AND},                 // and
    {"\\|\\|", TK_OR},              // or
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
// This is function is be called at firstly in the init_monitor().
void init_regex() {
    char error_msg[128];

    for (int i = 0; i < NR_REGEX; i++) {
        int ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg,
                  rules[i].regex);
        }
    }
}

// Here we hard code the max limit of num length, 32.
typedef struct token {
    int  type;
    char str[32];
} Token;

// Here we hard code the max limit of tokens, 6700.
static Token tokens[6700] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char* e) {
    regmatch_t pmatch;
    int i, position = 0;

    // everytime we evluate the expression, we need rematch the tokens.
    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            // int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
            // Here, we need the RTFM.
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                /*
                 *typedef struct {
                 *    regoff_t rm_so;
                 *    regoff_t rm_eo;
                 *} regmatch_t;
                 */
                char* substr_start = e + position;
                int   substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: "
                    "%.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add
                 * codes to record the token in the array `tokens'. For
                 * certain types of tokens, some extra actions should be
                 * performed.
                 */

                Assert(substr_len < 32, "expression length too long");

                switch (rules[i].token_type) {
                /* TODO: Insert codes to construct the tokens array. */
                case TK_NOTYPE:
                    break;
                case TK_DEC:
                case TK_HEX:
                case TK_REG:
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                    // Here, we delibreately straightly fallthrough.
                default:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                }

                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e,
                   position, "");
            return false;
        }
    }

    return true;
}

static bool is_operator(int opt_type){
    if(opt_type == ')' || opt_type == TK_DEC || opt_type == TK_HEX || opt_type == TK_REG)
        return false;
    return true;
}

static uint32_t get_val(int index){
    uint32_t val = 0;
    switch(tokens[index].type){
    // strtol and sscanf two functions are all suitable.
    case TK_DEC:
        if(sscanf(tokens[index].str, "%d", &val) == 1)
            return val;
    case TK_HEX:
        if(sscanf(tokens[index].str, "%x", &val) == 1)
            return val;
    }
    assert("get_eval error");
    return -1;
}

static bool check_parentheses(int left, int right){
    if(tokens[left].type != '(' || tokens[right].type != ')')
        return false;

    int top = 0;
    for(int i = left+1; i < right; ++i){
        // Here is some stack tricks to judge the parethesis with pair.
        if (tokens[i].type == '('){
            top++;
        } else if (tokens[i].type == ')') {
            top--;
            // With the left+1 as start point, the ')' reject the separation parethesis pairs, which we need eval.
            if(top < 0){
                return false;
            }
        }
    }

    return true;
}

static int get_priority(int opt_type) {
    switch (opt_type) {
    case TK_NEG:
    case TK_DEREF:
        return 6;
    case '*':
    case '/':
        return 5;
    case '+':
    case '-':
        return 4;
    case TK_EQ:
    case TK_NEQ:
        return 3;
    case TK_AND:
        return 2;
    case TK_OR:
        return 1;
    }
    panic("get_priority failed");
    return -1;
}

static int get_main_operator(int left, int right){
    int pos = left;
    int priority = 7;
    int parenthesis = 0;

    for(int i = left; i <= right; ++i){
        if(tokens[i].type == '('){
            parenthesis++;
            continue;
        } else if(tokens[i].type == ')'){
            parenthesis--;
            if (parenthesis < 0)
                panic("get_main_operator error");
            continue;
        }


        if(is_operator(tokens[i].type) && !parenthesis) {
            int level = get_priority(tokens[i].type);
            // Here is for uni-op and bi-op the associativity
            if(level < 6 && level <= priority){
                pos = i;
                priority = level;
            } else if (level == 6 && level < priority){
                pos = i;
                priority = level;
            }
        }
    }

    return pos;
}

static uint32_t eval(int left, int right) {
    //printf("left == %d, right == %d\n", left, right);
    if (left > right) {
        /* Bad expression */
        //printf("error: left == %d, right == %d\n", left, right);
        panic("eval failed");
    } else if (left == right) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */
        uint32_t val = 0;
        if (tokens[left].type == TK_DEC || tokens[left].type == TK_HEX) {
            val = get_val(left);
        } else if(tokens[left].type == TK_REG){
            bool flag = false;
            val = isa_reg_str2val(tokens[left].str+1, &flag);
            Assert(flag == true, "eval failed");
        }
        return val;
    } else if (check_parentheses(left, right) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        return eval(left + 1, right - 1);
    } else {
        int op = get_main_operator(left, right);
        //printf("op == %d %d %c\n", op, tokens[op].type, tokens[op].type);

        if(tokens[op].type == TK_NEG){
            return -eval(op+1, right);
        }
        if(tokens[op].type == TK_DEREF)
            return vaddr_read(eval(op+1, right), 4);

        int val1 = eval(left, op - 1);
        int val2 = eval(op + 1, right);

        switch (tokens[op].type) {
        case '+':
            return val1 + val2;
        case '-':
            return val1 - val2;
        case '*':
            return val1 * val2;
        case '/':
            //if(val2 == 0)
            //    panic("eval fail for division by 0");
            return val1 / val2;
        case TK_EQ:
            return (uint32_t)(val1 == val2);
        case TK_NEQ:
            return (uint32_t)(val1 != val2);
        case TK_AND:
            return (uint32_t)(val1 && val2);
        case TK_OR:
            return (uint32_t)(val1 || val2);
        default:
            assert(0);
        }
    }
}

word_t expr(char* e, bool* success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    for (int i = 0; i < nr_token; ++i) {
        if (tokens[i].type == '-' &&
            (i == 0 || is_operator(tokens[i - 1].type))) {
            tokens[i].type = TK_NEG;
        }
        if (tokens[i].type == '*' &&
            (i == 0 || is_operator(tokens[i - 1].type))) {
            tokens[i].type = TK_DEREF;
        }
    }

    *success = true;
    return eval(0, nr_token-1);
}
