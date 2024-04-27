#include <am.h>
#include <NDL.h>

#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define keyname(k) #k,

static const char *keyname[] = {
    "NONE",
    _KEYS(keyname)
};

typedef void (*handler_t)(void* buf);

void __am_timer_config(AM_TIMER_CONFIG_T* cfg){
    cfg->present = true;
    cfg->has_rtc = true;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T* uptime){
    struct timeval tv;
    assert(!gettimeofday(&tv, NULL));
    uptime->us = tv.tv_sec * 1000000 + tv.tv_usec;
}

void __am_timer_rtc(AM_TIMER_RTC_T* rtc){
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}

void __am_input_config(AM_INPUT_CONFIG_T* cfg){
    cfg->present = true;
}

void __am_input_keybrd(AM_INPUT_KEYBRD_T* kbd) {
    unsigned buf_size = 64;
    char*    buf = (char*)malloc(buf_size * sizeof(char));
    assert(buf);
    memset(buf, 0, buf_size);

    int rv = NDL_PollEvent(buf, sizeof(buf));

    if (rv > 0) {
        if (strncmp(buf, "kd", 2) == 0) {
            kbd->keydown = 1;
        } else {
            kbd->keydown = 0;
        }

        for (unsigned i = 0; i < sizeof(keyname) / sizeof(keyname[0]);
             ++i) {
            if (strncmp(buf + 3, keyname[i], strlen(buf) - 4) == 0 &&
                strlen(keyname[i]) == strlen(buf) - 4) {
                kbd->keycode = i;
                break;
            }
        }
    } else {
        kbd->keydown = 0;
        kbd->keycode = AM_KEY_NONE;
    }
    free(buf);
}

static int gpu_sync = false;
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    int fd = open("/proc/dispinfo", O_RDONLY);
    assert(fd);

    unsigned buf_size = 64;
    char*    buf = (char*)malloc(buf_size * sizeof(char));
    assert(buf);
    memset(buf, 0, buf_size);
    read(fd, buf, sizeof(buf));

    int width, height;
    sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &width, &height);

    *cfg = (AM_GPU_CONFIG_T){
        .present = true,
        .has_accel = false,
        .width = width,
        .height = height,
        .vmemsz = width * height * sizeof(uint32_t),
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    int x = ctl->x;
    int y = ctl->y;
    int w = ctl->w;
    int h = ctl->h;

    AM_GPU_CONFIG_T cfg;
    __am_gpu_config(&cfg);
    int width = cfg.width;
    int height = cfg.height;

    uint32_t * base = (uint32_t *) ctl->pixels;

    int fd = open("/dev/fb", O_WRONLY);
    assert(fd);
    for (int i = 0; i < h && y + i < height; ++i) {
        lseek(fd, ((y + i) * width + x) * 4, SEEK_SET);
        write(fd, base + i * w, (w < width - x ? w : width - x) * 4);
    }

    if (ctl->sync) {
        gpu_sync = true;
    } else {
        gpu_sync = false;
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = gpu_sync;
}

static void* lut[16] = {
    [AM_TIMER_CONFIG] = __am_timer_config,
    [AM_TIMER_RTC] = __am_timer_rtc,
    [AM_TIMER_UPTIME] = __am_timer_uptime,
    [AM_INPUT_CONFIG] = __am_input_config,
    [AM_INPUT_KEYBRD] = __am_input_keybrd,
    [AM_GPU_CONFIG] = __am_gpu_config,
    [AM_GPU_STATUS] = __am_gpu_status,
    [AM_GPU_FBDRAW] = __am_gpu_fbdraw,
};

static inline void implement_me(void* buf){
    printf("%s reg is non-existence, please implement me", (char*)buf);
}

bool ioe_init() {
    for (int i = 0; i < LENGTH(lut); i++)
        if (!lut[i])
            lut[i] = implement_me;
    return true;
}

void ioe_read(int reg, void* buf) {
    ((handler_t)lut[reg])(buf);
}
void ioe_write(int reg, void* buf) {
    ((handler_t)lut[reg])(buf);
}
