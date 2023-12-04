#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define MAX_BUF 2048
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    static char ans[MAX_BUF] = {};
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(ans, MAX_BUF, fmt, ap);
    va_end(ap);
    for(int i = 0; ans[i]; ++i){
        putch(ans[i]); 
    }
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
    va_list arg;
    va_start(arg, fmt);
    int ret = vsnprintf(out, n, fmt, arg);
    va_end(arg);
    return ret;
}

int vsprintf(char* out, const char* fmt, va_list ap) {
    return vsnprintf(out, -1, fmt, ap);
}

void itoa(unsigned int n, char* buf) {
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
}

int vsnprintf(char* out, size_t n, const char* fmt, va_list ap) {
    size_t cnt = 0;
    bool flag = true;
    memset(out, 0, MAX_BUF);

    for (size_t num = 0; *fmt && flag ; ++fmt) {
        char cur = *fmt;
        // %[flags][width][.precision][length]specifier
        if (cur == '%') {
            ++fmt;
            char c = '\0';
            char *s = NULL;
            char buf[MAX_BUF/2] = {0};
            switch (*fmt) {
            case 'c':
                c = va_arg(ap, int);
                *out++ = c;
                cnt++;
                if(++num >= n)
                    flag = false;
                break;
            case 'd':
                n = va_arg(ap, int);
                if (n < 0){
                    num++;
                    n = -n;
                    *out++ = '-';
                }
                itoa(n, buf);
                memcpy(out, buf, strlen(buf));
                out += strlen(buf);
                num += strlen(buf);
                cnt++;
                if(++num >= n)
                    flag = false;
                break;
            // x is unsigned, no need to judge minus.
            case 'x':
                n = va_arg(ap, int);
                xtoa(n, buf);
                memcpy(out, buf, strlen(buf));
                out += strlen(buf);
                num += strlen(buf);
                cnt++;
                if(++num >= n)
                    flag = false;
                break;
            case 's':
                s = va_arg(ap, char*);
                memcpy(out, s, strlen(s));
                out += strlen(s);
                num += strlen(s);
                cnt++;
                if(++num >= n)
                    flag = false;
                break;
            case '%':
                *out++ = '%';
                cnt++;
                if(++num >= n)
                    flag = false;
                break;
            }
        } else {
            *out++ = cur;
            cnt++;
            if(++num >= n)
                flag = false; 
            break;  
        }
    }

    return cnt++;
}

#endif
