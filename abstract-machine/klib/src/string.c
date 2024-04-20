#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    int n;

    for (n = 0; s[n]; n++)
        ;
    return n;
}

char* strcpy(char* dst, const char* src) {
    char* os = dst;
    while ((*dst++ = *src++) != 0)
        ;
    return os;
}

char* strncpy(char* dst, const char* src, size_t n) {
    char* os = dst;
    while (n-- > 0 && (*dst++ = *src++) != 0)
        ;
    while (n-- > 0)
        *dst++ = 0;
    return os;
}

char *strcat(char *dst, const char *src) {
    char *os = dst;
    dst += strlen(dst);
    while(*src)
        *dst++ = *src++;
    *dst = '\0';
    return os;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2)
        s1++, s2++;
    if (!*s1)
        return 0;
    return (uint8_t)*s1 - (uint8_t)*s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n > 0 && *s1 && *s1 == *s2)
        n--, s1++, s2++;
    if (n == 0)
        return 0;
    return (uint8_t)*s1 - (uint8_t)*s2;
}

void* memset(void* dst, int c, size_t n) {
    char* cdst = (char*)dst;
    for (int i = 0; i < n; i++) {
        cdst[i] = c;
    }
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    const char* s;
    char*       d;

    if (n == 0)
        return dst;

    s = src;
    d = dst;
    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    } else
        while (n-- > 0)
            *d++ = *s++;

    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
    return memmove(out, in, n);
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t *t1 = s1, *t2 = s2;
    while (n-- > 0) {
        if (*t1 != *t2)
            return *t1 - *t2;
        t1++, t2++;
    }

    return 0;
}

#endif
