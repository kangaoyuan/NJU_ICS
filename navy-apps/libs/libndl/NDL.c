#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
static void init_dispinfo();
int open(const char* file, int flags, ...);

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

static struct timeval now;
static uint32_t       *canvas = NULL;
static int            canvas_w = 0, canvas_h = 0;
static int            canvas_relative_screen_w = 0, canvas_relative_screen_h = 0;
static FILE           *fb = NULL, *fb_event = NULL, *fb_sync = NULL, *fb_dispinfo = NULL;

static void get_display_info() {
    FILE* dispinfo = fopen("/proc/dispinfo", "r");
    assert(dispinfo);
    screen_w = screen_h = 0;
    char buf[128], key[128], value[128], *delim;
    while (fgets(buf, 128, dispinfo)) {
        *(delim = strchr(buf, ':')) = '\0';
        sscanf(buf, "%s", key);
        sscanf(delim + 1, "%s", value);
        if (strcmp(key, "WIDTH") == 0)
            sscanf(value, "%d", &screen_w);
        if (strcmp(key, "HEIGHT") == 0)
            sscanf(value, "%d", &screen_h);
    }
    fclose(dispinfo);
    assert(screen_w > 0 && screen_h > 0);
}


// unit: ms
uint32_t NDL_GetTicks() {
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000 + now.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  return 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 && *h == 0) {
      *w = screen_w;
      *h = screen_h;
  }

  canvas_w = *w;
  canvas_h = *h;

  assert(canvas == NULL);
  canvas = (uint32_t*)malloc(sizeof(uint32_t) * (*w) * (*h));
  memset(canvas, 0, sizeof(uint32_t) * (*w) * (*h));

  canvas_relative_screen_w = (screen_w - canvas_w) / 2;
  canvas_relative_screen_h = (screen_h - canvas_h) / 2;

  assert(canvas_w + canvas_relative_screen_w <= screen_w
          && canvas_h + canvas_relative_screen_h <= screen_h);

  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t* pixels, int x, int y, int w, int h) {
    FILE* fd = fopen("/dev/fb", 0);
    for (int i = 0; i < h && y + i < canvas_h; ++i) {
        fseek(fd, (y + canvas_relative_screen_h + i) * screen_w +
              (x + canvas_relative_screen_w), SEEK_SET);
        fwrite(pixels + i * w, 1, w < canvas_w - x ? w : canvas_w - x, fd);
    }
    assert(fclose(fd) == 0);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    now.tv_sec = now.tv_usec = 0;
    get_display_info();
    fb = fopen("/dev/fb", "w");
    fb_sync = fopen("/dev/sync", "w");
    fb_event = fopen("/dev/events", "r");
    return 0;
}

void NDL_Quit() {
    now.tv_sec = now.tv_usec = 0;
    free(canvas);
    fclose(fb_event);
    fclose(fb);
    fclose(fb_sync);
    fclose(fb_dispinfo);
}
