#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

/*
 * AM_AUDIO_CONFIG_T { bool present; int bufsize }
 * AM_AUDIO_CTRL_T   { int freq, channels, samples }
 * AM_AUDIO_STATUS_T { int count }
 * AM_AUDIO_PLAY_T   { Area buf }
 */

void __am_audio_init() {
    /* It be done inside the nemu to start up */
    //outl(AUDIO_SBUF_SIZE_ADDR, 1<<16);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);

  outl(AUDIO_COUNT_ADDR, 0);
  outl(AUDIO_INIT_ADDR, true);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  Area buf = ctl->buf;
  int len = buf.end - buf.start;
  uint8_t* audio_data = buf.start;
  int cnt = inl(AUDIO_COUNT_ADDR);
  int sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);

  while(len + cnt > sbuf_size){
    ;
    cnt = inl(AUDIO_COUNT_ADDR); 
  }

  uint8_t* audio_base = (uint8_t*)(uintptr_t)(AUDIO_SBUF_ADDR);
  for(int i = 0; i < len; ++i){
    audio_base[cnt+i] = audio_data[i]; 
  }

  outl(AUDIO_COUNT_ADDR, cnt + len);
}
