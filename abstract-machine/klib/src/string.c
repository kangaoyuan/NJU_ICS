#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char* s) {
    int cnt = 0;
    while (*s) {
        ++s;
        ++cnt;
    }
    return cnt;
}

char* strcpy(char* dst, const char* src) {
    char *result = dst;
    // while(*dst++ = *src++) ; // equalvalent to below two statements.
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
    return result;
}

char* strncpy(char* dst, const char* src, size_t n) {
    char *result = dst;
    while (*src && n > 0) {
        --n;
        *dst++ = *src++;
    }
    *dst = '\0';
    return result;
}

char* strcat(char* dst, const char* src) {
    char *result = dst;
    dst += strlen(dst);
    while(*src){
         *dst++ = *src++; 
    }
    *dst = '\0';
    return result;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 == *s2 && *s1 != '\0' && *s2 != '\0'){
        ++s1;
        ++s2;
    }
    return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while(n > 0){
        if(*s1 != *s2){
            return *s1 - *s2; 
        }
        --n;
        ++s1;
        ++s2;
    }
    return 0;
}

void* memset(void* v, int c, size_t n) {
    for(uint8_t *s = v; n > 0; ++s, --n){
        *s = c; 
    }
    return v;
}

void* memmove(void* dst, const void* src, size_t n) {
    uint8_t* udst = dst;
    const uint8_t* usrc = src;
    while(n > 0) {
        --n;
        *udst++ = *usrc++;
    }
    return dst;
}

void* memcpy(void* out, const void* in, size_t n) {
    for(uint8_t *dst = out; n > 0; --n){
        *dst++ = *(const uint8_t*)in++; 
    }
    return out;
}

int memcmp(const void* s1, const void* s2, size_t n) {
   const uint8_t* us1 = (const uint8_t*)s1;
   const uint8_t* us2 = (const uint8_t*)s2;
    while(n > 0){
        if(*us1 != *us2)
            return *us1 - *us2;
        --n;
        ++us1;
        ++us2;
    }
    return 0;
}

#endif
