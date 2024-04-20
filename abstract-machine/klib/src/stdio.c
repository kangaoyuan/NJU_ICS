#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define MAX_BUF 2048
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char digits[] = "0123456789ABCDEF";

static void printint(char** out, int xx, int base, int sgn) {
    char buf[32];
    unsigned int x;
    int  i = 0, neg = 0;

    if (sgn && xx < 0) {
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);
    if (neg)
        buf[i++] = '-';

    while (--i >= 0){
        **out = buf[i];
        ++(*out);
    }
}

static void printptr(char** out, uint32_t x) {
    int i;
    **out = '0';
    ++(*out);
    **out = 'x';
    ++(*out);
    for (i = 0; i < (sizeof(uint32_t) * 2); i++, x <<= 4){
        **out = digits[x >> (sizeof(uint32_t) * 8 - 4)];
        ++(*out);
    }
}

int printf(const char *fmt, ...) {
    if(!fmt)
        panic("null fmt in printf");

    va_list ap;
    char buf[MAX_BUF];
    va_start(ap, fmt);
    int rv = vsprintf(buf, fmt, ap);
    putstr(buf);
    va_end(ap);
    return rv;
}

int sprintf(char *out, const char *fmt, ...) {
    if(!fmt)
        panic("null fmt in printf");

    va_list ap;
    va_start(ap, fmt);
    int rv = vsprintf(out, fmt, ap);
    va_end(ap);
    return rv;
}

int vsprintf(char* out, const char* fmt, va_list ap) {
    char* s;
    int   c, state = 0, cnt = 0;

    for (int i = 0; fmt[i]; i++) {
        c = fmt[i] & 0xff;
        if (state == 0) {
            if (c == '%') {
                state = '%';
            } else {
                *out++ = c;
            }
        } else if (state == '%') {
            if (c == '%') {
                *out++ = c;
            } else {
                cnt++;
                if (c == 'd') {
                    printint(&out, va_arg(ap, int), 10, 1);
                } else if (c == 'l') {
                    printint(&out, va_arg(ap, unsigned long), 10, 0);
                } else if (c == 'x') {
                    printint(&out, va_arg(ap, int), 16, 0);
                } else if (c == 'p') {
                    printptr(&out, va_arg(ap, unsigned int));
                } else if (c == 's') {
                    s = va_arg(ap, char*);
                    if (s == 0)
                        s = "(null)";
                    while (*s != 0) {
                        *out++ = *s++;
                    }
                } else if (c == 'c') {
                    *out++ = va_arg(ap, unsigned int);
                } else {
                    // Unknown % sequence.  Print it to draw attention.
                    *out++ = '%';
                    *out++ = c;
                }
            }
            state = 0;
        }
    }
    *out = '\0';

    return cnt;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    if(!fmt)
        panic("null fmt in printf");

    va_list ap;
    va_start(ap, fmt);
    int rv = vsnprintf(out, n, fmt, ap);
    va_end(ap);
    return rv;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    // unimplement the n restriction.
    char* s;
    int   c, state = 0, cnt = 0;

    for (int i = 0; fmt[i]; i++) {
        c = fmt[i] & 0xff;
        if (state == 0) {
            if (c == '%') {
                state = '%';
            } else {
                *out++ = c;
            }
        } else if (state == '%') {
            cnt++;
            if (c == 'd') {
                printint(&out, va_arg(ap, int), 10, 1);
            } else if (c == 'l') {
                printint(&out, va_arg(ap, unsigned long), 10, 0);
            } else if (c == 'x') {
                printint(&out, va_arg(ap, int), 16, 0);
            } else if (c == 'p') {
                printptr(&out, va_arg(ap, unsigned int));
            } else if (c == 's') {
                s = va_arg(ap, char*);
                if (s == 0)
                    s = "(null)";
                while (*s != 0) {
                    *out++ = *s++;
                }
            } else if (c == 'c') {
                *out++ = va_arg(ap, unsigned int);
            } else if (c == '%') {
                *out++ = c;
            } else {
                // Unknown % sequence.  Print it to draw attention.
                *out++ = '%';
                *out++ = c;
            }
            state = 0;
        }
    }

    return cnt;
}

#endif
