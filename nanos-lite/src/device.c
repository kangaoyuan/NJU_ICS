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
    AM_INPUT_KEYBRD_T key = io_read(AM_INPUT_KEYBRD);
    int keycode = key.keycode;
    bool keydown = key.keydown;

    if(keycode == AM_KEY_NONE)
        return 0;

    memset(buf, 0, len);
    keydown ? sprintf(buf, "kd %s\n", keyname[keycode]) : sprintf(buf, "ku %s\n", keyname[keycode]);

    return strlen(buf);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
    int width = info.width;
    int height = info.height;

    memset(buf, 0, len);
    sprintf((char*)buf, "WIDTH:%d\nHEIGHT:%d\n", width, height);

	return strlen(buf);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
    int width = info.width;
    int height = info.height;
    assert(offset <= width * height * 4);

    int x = (offset/4) % width;
    int y = (offset/4) / width;

    if (offset + len > width * height * 4)
        len = width * height * 4 - offset;

    io_write(AM_GPU_FBDRAW, x, y, (void*)buf, len/4, 1, true);

    return len;
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
