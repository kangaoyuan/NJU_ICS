#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define MAX_BUF 2048
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

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

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}



int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
