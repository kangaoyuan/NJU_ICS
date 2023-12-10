#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFFER_SIZE (2048)

enum { zeropad = 1, align = 2, plus = 4, space = 8, prgiven = 16};

static int isdigit(char c) {
    return '0' <= c && c <= '9';
}

//static int islower(char c) {    
//    return 'a' <= c && c <= 'z';
//}

void itoa_base(long long dec, char *dest, int base) {
    assert(base == 2 || base == 8 || base == 10 || base == 16); 

    if(dec == 0){
        dest[0] = '0'; 
        dest[1] = '\0'; 
        return;
    }
    
    if(dec < 0){
        dec = -dec; 
        //*dest++ = '-';
    }

    int width = 0;
    for(int tmp = dec; tmp > 0; tmp /= base)
        width++;
    dest[width--] = '\0';
    for(int rem = dec % base; dec > 0; dec = dec / base, rem = dec % base)
        dest[width--] = rem > 9 ? rem - 10 + 'a' : rem + '0';
    assert(dec == 0 && width == -1);
}

void itoa(long long dec, char* dest) {
    itoa_base(dec, dest, 10);
}

void xtoa(long long dec, char* dest) {
    itoa_base(dec, dest, 16);
}

static void print_pad(char **at, size_t *left, int *ret, char pad, int num){
    while(num--) {
        if(*left > 1) {
            *(*at)++ = pad;
            (*left)--;
        }
        (*ret)++;
    }
}

static char get_negsign(bool neg, bool plus, bool space){
    if(neg)
        return '-';
    if(plus)
        return '+';
    if(space)
        return ' ';
    return 0;
}

static void spool_str(char** at, size_t* left, int* ret,
                      const char* buf, int len) {
	for(int i = 0; i < len; i++) {
		if(*left > 1) {
			*(*at)++ = buf[i];
			(*left)--;
		}
		(*ret)++;
	}
}

static int print_rem(char *buf, int max, double rem, int prec){
    if(prec > 19)
       prec = 19; 
    unsigned long long cap = 1;
    for(int i = 0; i < prec; ++i){
        cap *= 10; 
    }
    rem *= (double)cap;
    unsigned long long val = rem;
    
    int num = 0;
    for(unsigned long long tmp = val; tmp > 0; tmp /= 10)
        num++;

    int len = 0;
    buf[len++] = '.'; 
    while(num <= prec)
        buf[len++] = '0'; 
    itoa(val, buf + len);
    len += strlen(buf + len);
    return len;
}

static void print_num(char **at, size_t *left, int *ret,
                      int minw, int precision, int flag,
                      bool zero, bool neg, char *buf, int len){
    // sign is '-', '+' or ' ', which is as part of length.
    char sign = get_negsign(neg, flag & plus, flag & space);
    if(flag & align) {
        /* calc numw = [sign][zeroes][number] */
		int numw = len;
		if(precision == 0 && zero) 
            numw = 0;
		if(numw < precision) 
            numw = precision;

		/* sign */
		if(sign) {
            numw++;
            print_pad(at, left, ret, sign, 1);
        }
		/* number */
		if(precision != 0 || !zero) {
			if(len < precision)
				print_pad(at, left, ret, '0', precision - len);
		    spool_str(at, left, ret, buf, len);	
		}
		/* spaces */
		if(numw < minw)
			print_pad(at, left, ret, ' ', minw - numw);
    } else {
        /* pad on the left of the number */
		int numw = len;
		if(precision == 0 && zero)
            numw = 0;
		if(numw < precision) 
            numw = precision;

		if(!(flag & prgiven) && (flag & zeropad) && numw < minw) 
            numw = minw;
		else if(sign) 
            numw++;

		/* pad with spaces */
		if(numw < minw)
			print_pad(at, left, ret, ' ', minw - numw);
		/* print sign (and one less zeropad if so) */
		if(sign) {
			print_pad(at, left, ret, sign, 1);
			numw--;
		}
		/* pad with zeroes */
		if(len < numw)
			print_pad(at, left, ret, '0', numw - len);
		if(precision == 0 && zero)
			return;
		/* print the characters for the value */
		spool_str(at, left, ret, buf, len); 
    }
}

static void print_num_d(char** at, size_t* left, int* ret, int val,
                        int minw, int precision, int flag) {
    char buf[BUFFER_SIZE];
    itoa(val, buf);
    int len = strlen(buf);
    bool zero = (val == 0), neg = (val < 0);
    print_num(at, left, ret, minw, precision, flag, zero, neg, buf, len);
}

static void print_num_u(char** at, size_t* left, int* ret, unsigned int val
                        , int minw, int precision, int flag){
	char buf[BUFFER_SIZE];
    itoa(val, buf);
	int len = strlen(buf);
    bool zero = (val == 0), neg = false;
	print_num(at, left, ret, minw, precision, flag, zero, neg, buf, len);
}

static void print_num_x(char** at, size_t *left, int *ret, unsigned int val
                        , int minw, int precision, int flag){
    char buf[BUFFER_SIZE];
    xtoa(val, buf);
    int len = strlen(buf);
    bool zero = (val == 0), neg = false;
    print_num(at, left, ret, minw, precision, flag, zero, neg, buf, len);
}

static void print_num_p(char** at, size_t *left, int *ret, void* val
                        , int minw, int precision, int flag){
    char buf[BUFFER_SIZE];
    int len;
    bool zero = (val == 0), neg = false;
    if(zero) {
        buf[0] = '(';
        buf[1] = 'n';
        buf[2] = 'i';
        buf[3] = 'l';
        buf[4] = ')';
        len = 5;
    } else {
        buf[0] = '0';
        buf[1] = 'x';
        xtoa((uintptr_t)val, buf+2);
        len = 2 + strlen(buf);  
    }
    print_num(at, left, ret, minw, precision, flag, zero, neg, buf, len);
}


static void print_num_f(char **at, size_t *left, int *ret, double val,
                        int minw, int precision, int flag){
    char buf[BUFFER_SIZE];
    bool zero = false, neg = val < 0;
    if(!(flag & prgiven))
        precision = 6;
    if(neg)
        val = -val;

    unsigned long long whole = (unsigned long long)val;
	double remain = val - (double)whole;
    itoa(whole, buf);
    int len = strlen(buf);
    if(precision != 0) // %[width].not_postive_val -> set precision 0
        len = print_rem(buf + len, sizeof(buf) - len, remain, precision); 

    flag &= ~prgiven;
    print_num(at, left, ret, minw, 1, flag, zero, neg, buf, len);
}
static void print_num_g(char **at, size_t *left, int *ret, double val,
                        int minw, int precision, int flag){
    char buf[BUFFER_SIZE];
    bool zero = false, neg = val < 0;
    if(!(flag & prgiven))
        precision = 6;     
    if(precision == 0) // %[width].not_postive_val -> set precision 0
        precision = 1;
    if(neg)
        val = -val;

    unsigned long long whole = (unsigned long long)val;
	double remain = val - (double)whole;
    itoa(whole, buf);
	int len = strlen(buf);

	/* number of digits before the decimal point */
    int before = 0;
	for(unsigned long long tmp = whole; tmp > 0; tmp /= 10) 
		before++;

	if(precision > before && remain != 0.0) {
		/* see if the last decimals are zero, if so, skip them */
		len += print_rem(buf+len, sizeof(buf)-len, remain, precision-before);
        while(buf[strlen(buf)-1] == '0')
            buf[strlen(buf)-1] = '\0'; 
	}

    flag &= ~prgiven;
    print_num(at, left, ret, minw, 1, flag, zero, neg, buf, len);
}

static void print_s(char** at, size_t* left, int* ret, char *str,
                    int minw, int precision, int flag){
    // try name
    int width = strlen(str);
    bool dot = flag & prgiven;
    bool minus = flag & align;
    if(dot)
        width = precision < width ? precision : width;

    if(width < minw && !minus) 
        print_pad(at, left, ret, ' ', minw - width); 
    spool_str(at, left, ret, str, width); 
    if(width < minw && minus) 
        print_pad(at, left, ret, ' ', minw - width); 
}

static void print_c(char **at, size_t *left, int *ret, 
                    int c, int minw, int flag){
    bool minus  = flag & align;

    if(1 < minw && !minus)
        print_pad(at, left, ret, ' ', minw-1);
    print_pad(at, left, ret, c, 1);
    if(1 < minw && minus)
        print_pad(at, left, ret, ' ', minw-1);
}

int printf(const char *fmt, ...) {
    static char ans [BUFFER_SIZE] = {};
    va_list ap;
    va_start(ap, fmt);
    // type_obj = va_arg(ap, type);
    int ret = vsnprintf(ans, BUFFER_SIZE, fmt, ap);
    va_end(ap);
    putstr(ans);
    return ret;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vsprintf(out, fmt, ap);
    va_end(ap);
    return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(out, n, fmt, ap);
    va_end(ap);
    return ret;
}

int vsprintf(char* out, const char* fmt, va_list ap) {
    return vsnprintf(out, -1, fmt, ap);
}

int vsnprintf(char* out, size_t n, const char* fmt, va_list ap) {
    char *outp = out;
    int   ret = 0;

    while (*fmt) {
        if (*fmt != '%') {
            ret++;
            if (n > 1){
                *outp++ = *fmt++;
                n--; 
            } else{
                fmt++; 
            }
        } else {
            // %[flags][*|width][.precision][length]specifier[length]
            fmt++;
            int  flag = 0, mnWidth = 0, precision = 1;

            while (true) {
                if (*fmt == '0')
                    flag |= zeropad;
                else if (*fmt == '-')
                    flag |= align;
                else if (*fmt == '+')
                    flag |= plus;
                else if (*fmt == ' ')
                    flag |= space;
                else
                    break;
                ++fmt;
            }

            if (isdigit(*fmt)) {
                while (isdigit(*fmt))
                    mnWidth = mnWidth * 10 + ((*fmt++) - '0');
            } else if (*fmt == '*') {
                fmt++;
                mnWidth = va_arg(ap, int);
                if (mnWidth < 0) {
                    flag |= align;
                    mnWidth = -mnWidth;
                }
            }

            if (*fmt == '.') {
                fmt++;
                precision = 0;
                flag |= prgiven;
                if (isdigit(*fmt)) {
                    while (isdigit(*fmt))
                        precision = precision * 10 + ((*fmt++) - '0');
                } else if (*fmt == '*') {
                    fmt++;
                    precision = va_arg(ap, int);
                    if (precision < 0)
                        precision = 0;
                }
            }

            switch (*fmt++) {
            case 'i':
            case 'd': 
                print_num_d(&outp, &n, &ret, va_arg(ap, int), 
                          mnWidth, precision, flag); 
                break;
            case 'u':             
                print_num_u(&outp, &n, &ret, va_arg(ap, unsigned int), 
                          mnWidth, precision, flag); 
                break;
            case 'x': 
                print_num_x(&outp, &n, &ret, va_arg(ap, unsigned int), 
                          mnWidth, precision, flag); 
                break;
            case 'p': 
                print_num_p(&outp, &n, &ret, va_arg(ap, void *), 
                          mnWidth, precision, flag); 
                break;
            case 'f':             
                print_num_f(&outp, &n, &ret, va_arg(ap, double), 
                          mnWidth, precision, flag); 
                break;
            case 'g':             
                print_num_g(&outp, &n, &ret, va_arg(ap, double), 
                          mnWidth, precision, flag); 
                break;
            case 's': 
                print_s(&outp, &n, &ret, va_arg(ap, char *),
                          mnWidth, precision, flag);
                break;
            case 'c':
                print_c(&outp, &n, &ret, va_arg(ap, int),
                          mnWidth, flag);
                break;
            case '%':
                print_pad(&outp, &n, &ret, '%', 1);
                break;
            case '\0':
                break;
            default:
                assert(0);
                break;
            }
        }
    }
    if(n > 0)
        *outp = '\0';
    return ret;
}

#endif

/* void itoa(unsigned int n, char* buf) {
    if (n < 10) {
        buf[0] = n + '0';
        buf[1] = '\0';
        return;
    }

    itoa(n / 10, buf);

    int i = strlen(buf);
    buf[i] = (n % 10) + '0';
    buf[i + 1] = '\0';
}

void xtoa(unsigned int n, char* buf) {
    if (n < 10) {
        buf[0] = n + '0';
        buf[1] = '\0';
    } else if (n < 16) {
        buf[0] = n - 10 + 'a';
        buf[1] = '\0';
    }
    return;

    xtoa(n / 16, buf);

    int i = strlen(buf);
    if (n % 16 < 10) {
        buf[i] = (n % 16) + '0';
        buf[i + 1] = '\0';
    } else if (n < 16) {
        buf[i] = (n % 16) - 10 + 'a';
        buf[i + 1] = '\0';
    }
} */
