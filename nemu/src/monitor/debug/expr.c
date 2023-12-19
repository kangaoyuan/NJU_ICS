#include <isa.h>
#include <memory/vaddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions. */
#include <regex.h>
#include <stdlib.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,
    TK_NOEQ,
    /* TODO: Add more token types */
    TK_NUM,
    TK_HEX,
    TK_REG,
    TK_OR,
    TK_AND,
    TK_NEG,
    TK_DEREF,
};
static struct rule {
    const char* regex;
    int   token_type;
} rules[] = {
    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules. */
    {" +",  TK_NOTYPE}, // spaces
    {"\\(", '('},       // left
    {"\\)", ')'},       // right
    {"\\*", '*'},       // multiple
    {"/",   '/'},       // divide
    {"\\+", '+'},       // plus
    {"-",   '-'},       // subtract
    {"==",  TK_EQ},     // equal
    {"!=",  TK_NOEQ},   // not_equal
    { "&&", TK_AND }, // and
    { "\\|\\|", TK_OR },  // or
    // because 0x prefix, 0 is also a number. so the sequence is important.
    { "0[Xx][0-9A-Fa-f]+", TK_HEX }, // hex
    { "[0-9]+", TK_NUM }, // decimal
    { "\\$[A-Za-z]+", TK_REG }, // register
};
#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )


static regex_t re[NR_REGEX] = {};
/* Rules are used for many times.
 * Therefore we compile them using regcomp() only once before any usage. */
void init_regex() {
    int ret;
    char error_msg[128];

    for (int i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg,
                  rules[i].regex);
        }
    }
}

typedef struct token {
    int  type;
    char str[32];
} Token;
static int   nr_token __attribute__((used)) = 0;
static Token tokens[32] __attribute__((used)) = {};

static bool make_token(char* expr) {
    nr_token = 0;
    regmatch_t pmatch;
    int i = 0, position = 0;

    while (expr[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], expr + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                char* substr_start = expr + position;
                int   substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: "
                    "%.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);

                /* TODO: Now a new token is recognized with rules[i]. Add
                 * codes to record the token in the array `tokens'. For
                 * certain types of tokens, some extra actions should be
                 * performed. */
                switch (rules[i].token_type) {
                case '(':
                case ')':
                case '*':
                case '/':
                case '+':
                case '-':
                case TK_EQ:
                case TK_NOEQ:
                case TK_AND:
                case TK_OR:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case TK_NUM:
                case TK_HEX:
                case TK_REG:
                    tokens[nr_token].type = rules[i].token_type;
                    if (substr_len >= 32) assert(0);
                    memcpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token++].str[substr_len] = '\0';
                    break;
                }
                position += substr_len;
                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, expr,
                   position, "");
            return false;
        }
    }

    return true;
}

bool check_parentheses(int p, int q) {
    int  num = 0;
    bool inner = false;
    bool st = (tokens[p].type == '(');
    bool ed = (tokens[q - 1].type == ')');
    for (int i = p; i < q; ++i) {
        switch (tokens[i].type) {
        case '(':
            num++;
            break;
        case ')':
            num--;
            break;
        default:
            break;
        }
        inner |= ((num == 0) && (i != q - 1));
    }
    bool sum = (num == 0);
    assert(sum == true);
    return !inner && st && ed && sum;
}

int primary_op(int p, int q) {
    int num = 0, tag = true;
    int index = -1, priority = 11;
    for (int i = p; i < q; ++i) {
        // Only when num == 0, we can get the primary_op index(last op).
        switch (tokens[i].type) {
        case '(':
            num++;
            break;
        case ')':
            num--;
            break;
        case TK_NEG:
        case TK_DEREF:
            if (num == 0 && priority >= 10 && tag) {
                index = i;
                priority = 10;
                tag = false;
            }
            break;
        case '*':
        case '/':
            if (num == 0 && priority >= 9) {
                index = i;
                priority = 9;
            }
            break;
        case '+':
        case '-':
            if (num == 0 && priority >= 8) {
                index = i;
                priority = 8;
            }
            break;
        case TK_EQ:
        case TK_NOEQ:
            if (num == 0 && priority >= 7) {
                index = i;
                priority = 7;
            }
            break;
        case TK_AND:
            if (num == 0 && priority >= 6) {
                index = i;
                priority = 6;
            }
            break;
        case TK_OR:
            if (num == 0 && priority >= 5) {
                index = i;
                priority = 5;
            }
            break;
        default:
            break;
        }
    }
    assert(index != -1);
    return index;
}

uint32_t eval(int p, int q) {
    while (tokens[p].type == TK_NOTYPE)
        p++;
    while (tokens[q - 1].type == TK_NOTYPE)
        q--;

    if (p >= q) {
        /* Bad expression */
        panic("Bad expression");
    } else if (p + 1 == q) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number. */
        char* ptr;
        /* If endptr is not NULL, strtol(nptr, endptr, base);, which stores the address of the first invalid character in *endptr. */
        switch (tokens[p].type) {
        case TK_NUM:
            return strtol(tokens[p].str, &ptr, 10);
        case TK_HEX:
            return strtol(tokens[p].str, &ptr, 16);
        case TK_REG: {
            bool   success;
            word_t result = isa_reg_str2val(tokens[p].str + 1, &success);
            if (success)
                return result;
            else
                assert(0);
        }
        default:
            assert(0);
        }
    } else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses. */
        return eval(p + 1, q - 1);
    } else {
        /* inner is feasible |= true, so we should split expression. */
        int      op = primary_op(p, q);
        uint32_t val1 = 0, val2 = 0;
        if(tokens[op].type != TK_DEREF && tokens[op].type != TK_NEG)
            val1 = eval(p, op);
        val2 = eval(op + 1, q);

        switch (tokens[op].type) {
        case '+':
            return val1 + val2;
        case '-':
            return val1 - val2;
        case '*':
            return val1 * val2;
        case '/':
            if (val2 == 0)
                assert(0);
            return val1 / val2;
        case TK_EQ:
            return val1 == val2;
        case TK_NOEQ:
            return val1 != val2;
        case TK_AND:
            return val1 && val2;
        case TK_OR:
            return val1 || val2;
        case TK_DEREF: {
            /*uint32_t res = 0;
            uint8_t* base = (uint8_t*)guest_to_host(val2);
            for (int i = 3; i >= 0; --i)
                res = res * 256 + (*(base + i));
            return res;*/
            uint32_t res = vaddr_read(val2, 4);
            return res;
        }
        case TK_NEG:
            return -val2;
        default:
            assert(0);
        }
    }
    return 0;
}

word_t expr(char* expr, bool* success) {
    if (!make_token(expr)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    for (int i = 0; i < nr_token; ++i) {
        if (tokens[i].type == '*' && i == 0) {
            // first token is '*', and the ' ' white spaces is not added. 
            tokens[i].type = TK_DEREF;
        } else if (tokens[i].type == '*' && i >= 1) {
            // if '*' token previous token is a binary operator, as TK_DEREF
            if (tokens[i - 1].type == '+' || tokens[i - 1].type == '-' ||
                tokens[i - 1].type == '*' || tokens[i - 1].type == '/' ||
                tokens[i - 1].type == TK_EQ ||
                tokens[i - 1].type == TK_NOEQ ||
                tokens[i - 1].type == TK_AND ||
                tokens[i - 1].type == TK_OR ||
                tokens[i - 1].type == TK_DEREF ||
                tokens[i - 1].type == '(') {
                tokens[i].type = TK_DEREF;
            }
        }
    }

    for (int i = 0; i < nr_token; ++i) {
        if (tokens[i].type == '-' && i == 0) {
            tokens[i].type = TK_NEG;
        } else if (tokens[i].type == '-' && i >= 1) {
            if (tokens[i - 1].type == '+' || tokens[i - 1].type == '-' ||
                tokens[i - 1].type == '*' || tokens[i - 1].type == '/' ||
                tokens[i - 1].type == TK_EQ ||
                tokens[i - 1].type == TK_NOEQ ||
                tokens[i - 1].type == TK_AND ||
                tokens[i - 1].type == TK_OR ||
                tokens[i - 1].type == TK_NEG ||
                tokens[i - 1].type == '(') {
                tokens[i].type = TK_NEG;
            }
        }
    }

    *success = true;
    return eval(0, nr_token);
}
