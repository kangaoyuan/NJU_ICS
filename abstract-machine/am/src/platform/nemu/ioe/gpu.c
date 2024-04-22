#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
<<<<<<< HEAD
    // Below code is for testing.
    int       w = inw(VGACTL_ADDR + 2);  // TODO: get the correct width
    int       h = inw(VGACTL_ADDR);  // TODO: get the correct height
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    for (int i = 0; i < w * h; i++)
=======
    int       i;
    int       w = inw(VGACTL_ADDR + 2);  // TODO: get the correct width
    int       h = inw(VGACTL_ADDR);  // TODO: get the correct height
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    for (i = 0; i < w * h; i++)
>>>>>>> 52e2e2b (>  compile)
        fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T* cfg) {
    *cfg = (AM_GPU_CONFIG_T){.present = true,
                             .has_accel = false,
                             .width = inw(VGACTL_ADDR + 2),
                             .height = inw(VGACTL_ADDR),
<<<<<<< HEAD
                             .vmemsz = cfg->width * cfg->height * 4};
=======
                             .vmemsz = 0};
>>>>>>> 52e2e2b (>  compile)
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T* ctl) {
    uint32_t* ff;
    uint32_t width = inw(VGACTL_ADDR+2);
    uint32_t* pixels = (uint32_t*)ctl->pixels;
    uint32_t* fb = (uint32_t*)(uintptr_t)FB_ADDR;
    int       x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;


<<<<<<< HEAD
    // Here I'm not accounting for the edge problem.
=======
>>>>>>> 52e2e2b (>  compile)
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
<<<<<<< HEAD
}

// It's only for learning the workflow
void __am_gpu_memcpy(AM_GPU_MEMCPY_T *cpy){
    
=======
>>>>>>> 52e2e2b (>  compile)
}
