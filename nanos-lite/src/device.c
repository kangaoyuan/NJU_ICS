#include <common.h>
#include <sys/time.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    const char* p = (const char*)buf;
    for (int i = 0; i < len; ++i, ++p)
        putch(*p);
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
    int keycode = io_read(AM_INPUT_KEYBRD).keycode;
    bool keydown = io_read(AM_INPUT_KEYBRD).keydown;

    if(keycode == AM_KEY_NONE)
        return 0;

    memset(buf, 0, len);
    keydown ? sprintf(buf, "kd %s\n", keyname[keycode]) : sprintf(buf, "ku %s\n", keyname[keycode]);

    return strlen(buf);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

int sys_gettimeofday(struct timeval* tv, struct timezone* tz){
    uint64_t us = io_read(AM_TIMER_UPTIME).us;
    assert(tv);
    tv->tv_sec = us / 1000000;
    tv->tv_usec = us - us / 1000000 * 1000000;
    return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
