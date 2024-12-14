#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h> // IWYU pragma: keep
#include <assert.h>
#include <monitor/log.h>

// How to record the __FILE__, __LINE__ and __func__ in the source files.
// __VA_ARGS__ is a special macro for variadic macros
#define Log(format, ...) \
    _Log("\33[1;34m" "[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

// do{} while(0) tech to encapsulate the macro's content to a single block,
// especially for multi statements.
#define Assert(cond, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      fprintf(stderr, "\33[1;31m"); \
      fprintf(stderr, "Error: " __VA_ARGS__); \
      fprintf(stderr, "\33[0m\n"); \
      extern void isa_reg_display(); \
      extern void monitor_statistic(); \
      isa_reg_display(); \
      monitor_statistic(); \
      assert(cond); \
    } \
  } while (0)

// assert(0) with panic("") description;
#define panic(...) Assert(0, __VA_ARGS__)

#define TODO() panic("please implement me")

#endif // __DEBUG_H__
