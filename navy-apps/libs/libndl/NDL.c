#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

uint32_t NDL_GetTicks() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
    memset(buf, 0, len);
    int fd = open("/dev/events", 0, 0);
    int ret = read(fd, buf, len);
    close(fd);
    return ret == 0 ? 0 : 1;
}

void NDL_OpenCanvas(int* w, int* h) {
    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w;
        screen_h = *h;
        char buf[64];
        int  len = sprintf(buf, "%d %d", screen_w, screen_h);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1) {
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0)
                continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0)
                break;
        }
        close(fbctl);
    }

    char buf[64] = {0};
    int fd_disp = open("/proc/dispinfo", 0, 0);
    read(fd_disp, buf, sizeof(buf));
    sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h);
    if (*w > screen_w || *h > screen_h) {
        printf("Canvas size out of screen.\n");
        assert(0);
    }
    if (*w == 0 && *h == 0) {
        *w = screen_w;
        *h = screen_h;
    }
    canvas_w = *w;
    canvas_h = *h;
    canvas_x = (screen_w - canvas_w) / 2;
    canvas_y = (screen_h - canvas_h) / 2;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    int fd_fb = open("/dev/fb", 0, 0);

    for (int i = 0; i < h && i + y < canvas_h; i++) {
        int offset = (canvas_y + y + i) * screen_w + (canvas_x + x);
        lseek(fd_fb, 4 * offset, SEEK_SET);

        w = canvas_w - x > w ? canvas_w - x : w;
        write(fd_fb, pixels + i * w, 4 * w);
    }

    //close(fd_fb);
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
  return 0;
}

void NDL_Quit() {
}
