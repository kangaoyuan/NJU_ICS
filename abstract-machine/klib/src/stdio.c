#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFFER_SIZE (2048)

static int isdigit(char c){
  return '0' <= c && c <= '9';
}

static int islower(char c){
  return 'a' <= c && c <= 'z';
}

void itoa_base(int dec, char *dest, int base) {
    assert(base == 2 || base == 8 || base == 10 || base == 16); 

    if(dec == 0){
        dest[0] = '0'; 
        dest[1] = '\0'; 
        return;
    }
    
    if(dec < 0){
        dec = -dec; 
        *dest++ = '-';
    }

    int width = 0;
    for(int tmp = dec; tmp > 0; tmp /= base)
        width++;
    dest[width--] = '\0';
    for(int rem = dec % base; dec > 0; dec = dec / base, rem = dec % base)
        dest[width--] = rem > 9 ? rem - 10 + 'a' : rem + '0';
}

void itoa(int d, char* dest) {
    itoa_base(d, dest, 10);
}

void xtoa(int d, char* dest) {
    itoa_base(d, dest, 16);
}

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


int printf(const char *fmt, ...) {
    static char ans [BUFFER_SIZE] = {};
    va_list ap;
    va_start(ap, fmt);
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
    //size_t cnt = 0;
    //bool flag = true;
    //memset(out, 0, MAX_BUF);

    //for (size_t num = 0; *fmt && flag ; ++fmt) {
    //    char cur = *fmt;
    //    // %[flags][width][.precision][length]specifier
    //    if (cur == '%') {
    //        ++fmt;
    //        char c = '\0';
    //        char *s = NULL;
    //        char buf[MAX_BUF/2] = {0};
    //        switch (*fmt) {
    //        case 'c':
    //            c = va_arg(ap, int);
    //            *out++ = c;
    //            cnt++;
    //            if(++num >= n)
    //                flag = false;
    //            break;
    //        case 'd':
    //            n = va_arg(ap, int);
    //            if (n < 0){
    //                num++;
    //                n = -n;
    //                *out++ = '-';
    //            }
    //            itoa(n, buf);
    //            memcpy(out, buf, strlen(buf));
    //            out += strlen(buf);
    //            num += strlen(buf);
    //            cnt++;
    //            if(++num >= n)
    //                flag = false;
    //            break;
    //        // x is unsigned, no need to judge minus.
    //        case 'x':
    //            n = va_arg(ap, int);
    //            xtoa(n, buf);
    //            memcpy(out, buf, strlen(buf));
    //            out += strlen(buf);
    //            num += strlen(buf);
    //            cnt++;
    //            if(++num >= n)
    //                flag = false;
    //            break;
    //        case 's':
    //            s = va_arg(ap, char*);
    //            memcpy(out, s, strlen(s));
    //            out += strlen(s);
    //            num += strlen(s);
    //            cnt++;
    //            if(++num >= n)
    //                flag = false;
    //            break;
    //        case '%':
    //            *out++ = '%';
    //            cnt++;
    //            if(++num >= n)
    //                flag = false;
    //            break;
    //        }
    //    } else {
    //        *out++ = cur;
    //        cnt++;
    //        if(++num >= n)
    //            flag = false; 
    //        break;  
    //    }
    //}

    //return cnt++;
    static char  vsn_buffer[BUFFER_SIZE];
    static char* presentation = "0123456789abcdef";
    char*        buffer = vsn_buffer;
    char*        outp = out;
    int          ret = 0;
    const char*  ori = fmt;

    while (*fmt) {
        char cur = *fmt;
        if (cur != '%') {
            // every time out(outp) variable increments, the n is to judge
            *outp++ = cur;
            if (++ret == n)
                break;
        } else  {
            ++fmt;
            char pre = '\0';
            int  mnWidth = 0, outWidth = 0;
            char type = '\0';

            if (isdigit(*fmt)) {
                switch (*fmt) {
                case '0':
                    pre = '0';
                    fmt++;
                    break;
                default:
                    break;
                }
                while (isdigit(*fmt)) {
                    mnWidth = mnWidth * 10 + ((*fmt) - '0');
                    fmt++;
                }
            }
            if (islower(*fmt)) {
                type = *fmt;
            }
            char* cp = &buffer[BUFFER_SIZE - 1];
            *cp = '\0';
            switch (type) {
            case 'd': {
                int32_t var = va_arg(ap, int32_t);
                int     isneg = 0;
                if (var < 0) {
                    isneg = 1;
                    var = -var;
                }
                do {
                    *(--cp) = presentation[var % 10];
                    outWidth++;
                    var /= 10;
                } while (var);
                if (isneg) {
                    *(--cp) = '-';
                    outWidth++;
                }
                break;
            }
            case 'u': {
                uint32_t var = va_arg(ap, uint32_t);
                do {
                    *(--cp) = presentation[var % 10];
                    outWidth++;
                    var /= 10;
                } while (var);
                break;
            }
            case 'p': {
                // Do not use uint32_t uint64_t but use unsigned long to
                // support -m64 -m32
                unsigned long var = (unsigned long)(va_arg(ap, void*));
                do {
                    *(--cp) = presentation[var % 16];
                    outWidth++;
                    var /= 16;
                } while (var);
                *(--cp) = 'x';
                outWidth++;
                *(--cp) = '0';
                outWidth++;
                break;
            }
            case 'x': {
                uint32_t var = va_arg(ap, uint32_t);
                do {
                    *(--cp) = presentation[var % 16];
                    outWidth++;
                    var /= 16;
                } while (var);
                break;
            }
            case 'f': {
                double  var = va_arg(ap, double);
                int32_t _int = (int32_t)(var * 10000);  // remain 4 bit
                int     isneg = 0;
                if (_int < 0) {
                    isneg = 1;
                    _int = -_int;
                }
                for (int i = 0; i < 4; i++) {
                    *(--cp) = presentation[_int % 10];
                    outWidth++;
                    _int /= 10;
                }
                *(--cp) = '.';
                outWidth++;
                do {
                    *(--cp) = presentation[_int % 10];
                    outWidth++;
                    _int /= 10;
                } while (_int);
                if (isneg) {
                    *(--cp) = '-';
                    outWidth++;
                }
                break;
            }
            case 's': {
                char* sb = va_arg(ap, char*);
                char* se = sb;
                while (*se)
                    se++;
                while (se != sb) {
                    *(--cp) = *(--se);
                    outWidth++;
                }
                break;
            }
            default:
                putch('!');
                while (*ori) {
                    putch(*ori);
                    ori++;
                }
                assert(0);
                break;
            }
            if (pre != '\0') {
                while (outWidth < mnWidth) {
                    *(--cp) = pre;
                    outWidth++;
                }
            }
            while (*cp) {
                *outp = *cp;
                if (++ret == n)
                    break;
                outp++;
                cp++;
            }
        }  
        fmt++;
    }
    *outp = '\0';
    return ret;
}

#endif
