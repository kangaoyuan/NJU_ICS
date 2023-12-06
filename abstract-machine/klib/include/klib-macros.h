#ifndef KLIB_MACROS_H__
#define KLIB_MACROS_H__

#define LENGTH(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define ROUNDDOWN(a, sz)    ((((uintptr_t)a)) & ~((sz) - 1))
#define ROUNDUP(a, sz)      ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define IN_RANGE(ptr, area) ((area).start <= (ptr) && (ptr) < (area).end)
#define RANGE(st, ed)       (Area) { .start = (void *)(st), .end = (void *)(ed) }

#define STRINGIFY(s)        #s
#define _CONCAT(x, y)       x ## y
#define TOSTRING(s)         STRINGIFY(s)
#define CONCAT(x, y)        _CONCAT(x, y)

#define putstr(s) \
  ({ for (const char *p = s; *p; p++) putch(*p); })

#define io_read(reg) \
  ({ reg##_T __io_param; \
    ioe_read(reg, &__io_param); \
    __io_param; })

#define io_write(reg, ...) \
  ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ }; \
    ioe_write(reg, &__io_param); })

#define static_assert(const_cond) \
  static char CONCAT(_static_assert_, __LINE__) [(const_cond) ? 1 : -1] __attribute__((unused))

#define panic_on(cond, s) \
  ({ if (cond) { \
      putstr("AM Panic: "); putstr(s); \
      putstr(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
      halt(1); \
    } })
#define panic(s) panic_on(1, s)

#endif
