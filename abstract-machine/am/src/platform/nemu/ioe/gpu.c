#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
    /* int       w = inw(VGACTL_ADDR + 2);  // TODO: get the correct width
    int       h = inw(VGACTL_ADDR);  // TODO: get the correct height
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    for (int i = 0; i < w * h; i++)
        fb[i] = i;
    outl(SYNC_ADDR, 1); */
}

void __am_gpu_config(AM_GPU_CONFIG_T* cfg) {
    *cfg = (AM_GPU_CONFIG_T){.present = true,
                             .has_accel = false,
                             .width = inw(VGACTL_ADDR + 2),
                             .height = inw(VGACTL_ADDR),
                             .vmemsz = 0};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T* ctl) {
    uint32_t* ff;
    uint32_t width = inw(VGACTL_ADDR+2);
    uint32_t* pixels = (uint32_t*)ctl->pixels;
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    int       x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;


    for (int j = y; j < y + h; ++j)
        for (int i = x; i < x + w; ++i) {
            // printf("%u %x\n",*(ctl->pixels+p),FB_ADDR+i+j*320);
            ff = fb + (j * width);
            ff[i] = *pixels++;
        }

    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}
