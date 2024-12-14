#ifndef __MONITOR_LOG_H__
#define __MONITOR_LOG_H__

#include <stdio.h> // IWYU pragma: keep

#ifdef DEBUG
extern FILE* log_fp;
#	define log_write(...) \
  do { \
    extern bool log_enable(); \
    if (log_fp != NULL && log_enable()) { \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0)
#else
#	define log_write(...)
#endif

// Here, we don't use the ## to specify the __VA_ARGS__ for the assurance.
#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
    log_write(__VA_ARGS__); \
  } while (0)

void strcatf(char *buf, const char *fmt, ...);

#endif
