#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

// It's worth to learn the #define, #, ## methods
#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    // because stdout and stderr output to serial, streams (char devices) have no offset concept.
    for(int i = 0; i < len; ++i)
        putch(((char *)buf)[i]);
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if(ev.keycode == AM_KEY_NONE)
        return 0;

    if(ev.keydown)
        return snprintf(buf, len, "kd %s\n", keyname[ev.keycode]);
    else
        return snprintf(buf, len, "ku %s\n", keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T conf = io_read(AM_GPU_CONFIG);
    int width = conf.width, height = conf.height;
    return snprintf(buf, len, "WIDTH : %d\nHEIGHT : %d\n", width, height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T conf = io_read(AM_GPU_CONFIG);
    int width = conf.width;
    int y = offset / width;
    int x = offset - y * width;
    io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len, 1, true);
    return 0;
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}
