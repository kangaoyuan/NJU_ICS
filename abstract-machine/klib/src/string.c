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
    /*  char *result = dst;
        while(*dst++ = *src++) ; // equalvalent to below two statements.
        while (*src) {
            *dst++ = *src++;
        }
        *dst = '\0';
        return result;  */
    return memmove(dst, src, strlen(src)+1);
}

char* strncpy(char* dst, const char* src, size_t n) {
    char *result = dst;
    while (*src && n > 0) {
        --n;
        *dst++ = *src++;
    }
    while(n > 0){
        --n;
        *dst++ = '\0'; 
    }
    return result;
}

char* strcat(char* dst, const char* src) {
    /*  char *result = dst;
        dst += strlen(dst);
        while(*src){
             *dst++ = *src++; 
        }
        *dst = '\0';
        return result; */
    strcpy (dst + strlen (dst), src);
    return dst;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 == *s2 && *s1 != '\0'){
        ++s1;
        ++s2;
    }
    return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
    } while (--n4 > 0);
      n &= 3;
    }

  while (n > 0)
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
    return c1 - c2;
      n--;
    }

  return c1 - c2;
}

void* memset(void* v, int c, size_t n) {
    for(uint8_t *s = v; n > 0; ++s, --n){
        *s = c; 
    }
    return v;
}

void* memmove(void* dst, const void* src, size_t n) {
    unsigned char*       d = dst;
    const unsigned char* s = src;

    if (d < s || d >= s + n) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n - 1;
        s += n - 1;
        while (n--) {
            *d-- = *s--;
        }
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
   const uint8_t* us1 = s1;
   const uint8_t* us2 = s2;
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
