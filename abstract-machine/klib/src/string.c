#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;;
  while(s[i] != '\0') {i++;}
  return i;
}

char *strcpy(char* dst,const char* src) {
  size_t i = 0;
  while(src[i] != '\0')
  {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  size_t i;
  for(i = 0;i < n&& src[i] != '\0';i++)
    dst[i] = src[i];
  for(; i < n;i ++)
    dst[i] = '\0';
  return dst;
}

char* strcat(char* dst, const char* src) {
  size_t i = 0,j = 0;;
  while(dst[i] != '\0')
  {
    i++;
  }
  while(src[j] != '\0')
  {
    dst[i] = src[j];
    i++;
    j++;
  }
  dst[i] = '\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  char const *p1 = s1,*p2 = s2;
  while(*p1 == *p2 && *p1 != '\0' && *p2 != '\0')
  {
    //printf("%c %c\n",*p1,*p2);
    p1++;
    p2++;
  }
  return *p1-*p2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  assert(0);
  return 0;
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
