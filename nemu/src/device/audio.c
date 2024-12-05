#include <common.h>

#ifdef HAS_IOE

#include <device/map.h>
#include <SDL2/SDL.h>

#define AUDIO_PORT 0x200 // Note that this is not the standard
#define AUDIO_MMIO 0xa1000200
#define STREAM_BUF 0xa0800000
#define STREAM_BUF_MAX_SIZE 65536

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

// SDL callback function to fill audio data from MMIO STREAM_BUF to SDL.
static inline void audio_play(void *userdata, uint8_t *stream, int len) {
    static uint32_t rd_idx;
    SDL_memset(stream, 0, len);
    uint32_t cnt = audio_base[reg_count];
    uint32_t sbuf_size = audio_base[reg_sbuf_size];

    int nread = len > cnt ? cnt : len;
    if((rd_idx + nread) > sbuf_size){
        SDL_MixAudio(stream, sbuf + rd_idx, sbuf_size - rd_idx, SDL_MIX_MAXVOLUME); 
        SDL_MixAudio(stream + (sbuf_size - rd_idx), sbuf, nread - (sbuf_size - rd_idx), SDL_MIX_MAXVOLUME); 
    } else {
        SDL_MixAudio(stream, sbuf + rd_idx, nread, SDL_MIX_MAXVOLUME);    
    }
    rd_idx = (rd_idx + nread) % sbuf_size;

    audio_base[reg_count] -= nread;

    // It be done from the first statement.
    /*
     *if(len > cnt){
     *    memset(stream, 0, len - cnt); 
     *}
     */
}

static void init_audio_sdl(){
    SDL_AudioSpec s = {};
    s.format = AUDIO_S16SYS;
    s.userdata = NULL;

    s.freq = audio_base[reg_freq];
    s.channels = audio_base[reg_channels]; 
    s.samples = audio_base[reg_samples];
    s.callback = audio_play;

    int rc = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if(rc == 0) {
        SDL_OpenAudio(&s, NULL);
        SDL_PauseAudio(0);
    }
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
    if(audio_base[reg_init] == true){
        init_audio_sdl(); 
        audio_base[reg_init] = false;
    }
}

/*
 *static void subf_io_handler(uint32_t offset, int len, bool is_write) {
 *    if(is_write){
 *        audio_base[reg_count] += len; 
 *    }
 *}
 */

void init_audio() {
  uint32_t space_size = nr_reg * sizeof(uint32_t);
  audio_base = (void *)new_space(space_size);
  add_pio_map("audio", AUDIO_PORT, (void *)audio_base, space_size,
              audio_io_handler);
  add_mmio_map("audio", AUDIO_MMIO, (void *)audio_base, space_size,
               audio_io_handler);
  audio_base[reg_count] = 0;
  audio_base[reg_init] = false;
  audio_base[reg_sbuf_size] = STREAM_BUF_MAX_SIZE;

  sbuf = (void *)new_space(STREAM_BUF_MAX_SIZE);
  add_mmio_map("audio-sbuf", STREAM_BUF, (void *)sbuf, STREAM_BUF_MAX_SIZE,
               NULL);
  /*
   *add_mmio_map("audio-sbuf", STREAM_BUF, (void *)sbuf, STREAM_BUF_MAX_SIZE,
   *             sbuf_io_handler);
   */
}

#endif	/* HAS_IOE */
