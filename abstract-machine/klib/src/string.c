#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  while(s[i] != '\0')
      i++;
  return i;
}

char* strcpy(char* dst, const char* src) {
    size_t i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

int strcmp(const char* s1, const char* s2) {
    const char *p1 = s1, *p2 = s2;
    while (*p1 == *p2 && *p1 != '\0' && *p2 != '\0') {
        // printf("%c %c\n",*p1,*p2);
        p1++;
        p2++;
    }
    return *p1 - *p2;
}

char* strcat(char* dst, const char* src) {
    size_t i = strlen(dst), j = 0;
    while (src[j] != '\0') {
        dst[i++] = src[j++];
    }
    dst[i] = '\0';
    return dst;
}

size_t strnlen(const char*s, size_t n) {
    size_t i = 0;
    while(n && s[i]){
        i++;
        n--; 
    }
    return i;
}

char* strncpy(char* dst, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[i] = src[i];
    while (i < n)
        dst[i++] = '\0';
    return dst;
 /* size_t size = strnlen(src, n);
    if (size != n)
        memset(dst + size, '\0', n - size);
    return memcpy(dst, src, size); */
}

int strncmp(const char* s1, const char* s2, size_t n) {
    unsigned char c1 = '\0';
    unsigned char c2 = '\0';
    while (n > 0) {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0' || c1 != c2)
            return c1 - c2;
        n--;
    }
    return c1 - c2;
}

void* memset(void* s, int c, size_t n) {
    assert(s);
    for (int i = 0; i < n; i++) 
        ((unsigned char*)s)[i] = c;
    return s;
}

void* memmove(void* dst, const void* src, size_t n) {
    assert(dst && src);
    if (src > dst) {
        for (size_t i = 0; n--; ++i)
            ((unsigned char*)dst)[i] = *(unsigned char*)src++;
    } else {
        for (size_t i = n - 1; i >= 0; --i)
            ((unsigned char*)dst)[i] = *(unsigned char*)src++;
    }
    return dst;
}

void* memcpy(void* out, const void* in, size_t n) {
    assert(out && in);
    for (size_t i = 0; n--; ++i)
        ((unsigned char*)out)[i] = *(unsigned char*)in++;
    return out;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    assert(s1 && s2);
    size_t i = 0;
    while (((unsigned char*)s1)[i] == ((unsigned char*)s2)[i] && i < n) 
        i++;
    return i == n ? 0 : ((unsigned char*)s1)[i] - ((unsigned char*)s2)[i];
}

#endif
