#include <am.h>
#include <nemu.h>

#ifdef MODE_800x600
# define W 800
# define H 600
#else
# define W 400
# define H 300
#endif
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
    int       w = W;  // TODO: get the correct width
    int       h = H;  // TODO: get the correct height
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    for (int i = 0; i < w * h; i++)
        fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T* cfg) {
    *cfg = (AM_GPU_CONFIG_T){.present = true,
                             .has_accel = false,
                             .width = W,
                             .height = H,
                             .vmemsz = W * H * sizeof(uint32_t)};
}

#include <klib.h>
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T* ctl) {
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    int       x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    uint32_t* base = (uint32_t*)ctl->pixels;
    int       cp_bytes = sizeof(uint32_t) * (w < W - x ? w : W - x);
    for (int j = 0; j < h && y + j < H; ++j) {
        memcpy(&fb[(y + j) * W + x], base, cp_bytes);
        base += w;
    }
    if (ctl->sync) {
        outl(SYNC_ADDR, true);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T* status) {
    //status->ready = true;
    status->ready = (bool) inl(SYNC_ADDR);
}
