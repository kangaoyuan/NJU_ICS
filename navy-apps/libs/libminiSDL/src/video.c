#include <NDL.h>
#include <sdl-video.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/*
 *typedef struct SDL_Rect {
 *    int16_t  x, y;  // Position of the upper-left corner of the rectangle
 *    uint16_t w, h;  // The width and height of the rectangle
 *} SDL_Rect;
 */

/*
 *typedef struct SDL_Color{
 *    uint8_t r;
 *    uint8_t g;
 *    uint8_t b;
 *    uint8_t unused;
 *} SDL_Color;
 */

/*
 *typedef struct {
 *    int ncolors;
 *    SDL_Color* colors; // SDL_Color elements make up the palette
 *} SDL_Palette;
 */
// A SDL_Palette should never need to be created manually, it's automatically created when allocating
// a SDL_PixelFormat for a surface. If the BitsPerPixel > 8, the palette is NULL.

/*
 *typedef struct SDL_PixelFormat{
 *    SDL_Palette* palette;                           // Pointer to the paletter
 *    uint8_t      BitsPerPixel;                      // Bits used to represent each pixel in surface
 *    uint8_t      BytesPerPixel;                     // Bytes used to represent each pixel in surface
 *    uint8_t      Rloss, Gloss, Bloss, Aloss;
 *    uint8_t      Rshift, Gshift, Bshift, Ashift;
 *    uint32_t     Rmask, Gmask, Bmask, Amask;
 *    uint32_t     colorkey;                          // Pixel value of transparent pixels
 *    uint8_t      alph;                              // Overall surface alpha value
 *} SDL_PixelFormat;
 */
// A SDL_PixelFormat describes the foramt of the pixel data stord at the SDL_Surface.

// For 8-bit pixel formats, all pixels are represented by a uint8_t val which contains an index to 
// s->format->palette->colors, s->format->palette->colors[*(uint8_t*)s->pixels], s->format->palette->colors[*(uint8_t*)s->pixels]->[r|g|b]
// Above 8-bit Pixel formats are considered to be TrueColor formats and the color info is stored in the pixels themselves, not in a palette. 
/*
 *typedef struct SDL_Surface{
 *    int w, h;                   // width and height of the surface in pixels
 *    void* pixels;               // Pinter to the actual pixel data, when multi-thread, we need lock to do.
 *    int refcount;               // Used when freeing surface
 *    uint16_t pitch;             // length of a surface scanline in bytes
 *    uint32_t flags;             // surface flags
 *    SDL_Rect clip_rect;         // surface clip rectangle
 *    SDL_PixelFormat* format;    // pixels format
 *
 *} SDL_Surface;
 */



// This performs a fast blit from the src to the dst suface.
// The width and height in srcrect determine the size of the copied rect, only the position is used in the dstrect(the width and height are ignored)
// This assumes that the source and destination rectangles are the same size. If srcrect is NULL, the entire surface is copied. If dstrect is NULL, the destination position is (0, 0)(left-upper corner)
void SDL_BlitSurface(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst,
                     SDL_Rect* dstrect) {
    assert(dst && src);
    assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);

    // The width and height in srcrcet determine the size of copied rect
    int dst_x = !dstrect ? 0 : dstrect->x;
    int dst_y = !dstrect ? 0 : dstrect->y;
    int src_x = !srcrect ? 0 : srcrect->x;
    int src_y = !srcrect ? 0 : srcrect->y;
    int width = !srcrect ? src->w : srcrect->w;
    int height = !srcrect ? src->h : srcrect->h;

    // Here, I'm coding conservatively, we can ensure the constraint in the
    // upper caller.
    width = width < (dst->w - dst_x) ? width : (dst->w - dst_x);
    height = height < (dst->h - dst_y) ? height : (dst->h - dst_y);

    if (src->format->BitsPerPixel == 32) {
        uint32_t* data = (uint32_t*)src->pixels;
        uint32_t* base = (uint32_t*)dst->pixels;

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                base[(dst_y + i) * dst->w + dst_x + j] =
                    data[(src_y + i) * src->w + src_x + j];
            }
        }

    } else if (src->format->BitsPerPixel == 8) {
        uint8_t* data = (uint8_t*)src->pixels;
        uint8_t* base = (uint8_t*)dst->pixels;

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                base[(dst_y + i) * dst->w + dst_x + j] =
                    data[(src_y + i) * src->w + src_x + j];
            }
        }
    }

    return;
}

// This performs a fast fill of the given rectangle with some color.
// If dstrect is NULL, the whole surface will be filled with color pixel format.
void SDL_FillRect(SDL_Surface* dst, SDL_Rect* dstrect, uint32_t color) {
    assert(dst);
    int rec_x = !dstrect ? 0 : dstrect->x;
    int rec_y = !dstrect ? 0 : dstrect->y;
    int rec_w = dstrect ? (dstrect->w < (dst->w - dstrect->x)
                               ? dstrect->w
                               : (dst->w - dstrect->x))
                         : dst->w;
    int rec_h = dstrect ? (dstrect->h < (dst->h - dstrect->y)
                               ? dstrect->h
                               : (dst->h - dstrect->y))
                        : dst->h;


    assert(dst->format);
    if (dst->format->BitsPerPixel == 32) {
        uint32_t* base = (uint32_t*)dst->pixels;

        for (int i = 0; i < rec_h; ++i) {
            for (int j = 0; j < rec_w; ++j) {
                base[(rec_y + i) * dst->w + rec_x + j] = color;
            }
        }

    } else if (dst->format->BitsPerPixel == 8) {
/*
 *        SDL_Color* target = &(dst->format->palette->colors[255]);
 *        target->a = (color >> 24) & 0xff;
 *        target->r = (color >> 16) & 0xff;
 *        target->g = (color >> 8) & 0xff;
 *        target->b = (color) & 0xff;
 *
 *        for (int i = rec_y; i < rec_h; ++i) {
 *            for (int j = rec_x; j < rec_w; ++j)
 *                dst->pixels[i * dst->w + j] = 255;
 *        }
 */
        int index = -1;
        for(int i = 0; i < dst->format->palette->ncolors; ++i){
            if(color == dst->format->palette->colors[i].val) {
                index = i;
                break; 
            }
        }
        uint8_t* pixels = (uint8_t*)dst->pixels;
        for (int i = rec_y; i < rec_h; ++i) {
            for (int j = rec_x; j < rec_w; ++j)
                pixels[i * dst->w + j] = index;
        }
    }
}

// Makes sure the given area is updated on the given scrren. please RTFM 
void SDL_UpdateRect(SDL_Surface* screen, int x, int y, int w, int h) {
    printf("Update screen\n");
    if (x == 0 && y == 0 && w == 0 && h == 0) {
        w = screen->w;
        h = screen->h;
    }
    if(w > screen->w) w = screen->w;
    if(h > screen->h) h = screen->h;

    if (!screen->format->palette) {
        printf("32 bits Update screen\n");
        assert(screen->format->BitsPerPixel == 32);
        NDL_DrawRect((uint32_t*)screen->pixels, x, y, w, h);
        printf("Ending 32 bits Update screen\n");
    } else {
        printf("8 bits Update screen\n");
        uint8_t*  pixels_index = screen->pixels;
        printf("Malloc 8 bits Update screen\n");
        uint32_t* pixels = malloc(screen->w * screen->h * sizeof(uint32_t));
        assert(pixels);

        for (int i = 0; i < h; ++i) {
            for(int j = 0; j < w; j++){
                printf("Get 8 bits pixels\n");
                int ref = (y * screen->w + x) + (i * screen->w + j);
                printf("Get 8 bits pixels, ref == %d\n", ref);
                printf("before color");
                SDL_Color color =
                    screen->format->palette->colors[pixels_index[ref]];
                printf("after color");
                printf("Get 8 bits color, a == %d, r == %d, g == %d, b == %d\n", color.a, color.r, color.g, color.b);
                uint32_t p_elem =
                    color.a << 24 | color.r << 16 | color.g << 8 | color.b;
                printf("Get 8 bits pixels, p_elem == %d\n", p_elem);
                pixels[i*w + j] = p_elem;
            }
        }
        printf("Over SDL 8 bits Update screen\n");

        NDL_DrawRect(pixels, x, y, w, h);
        free(pixels);
        printf("Ending 8 bits Update screen\n");
    }
    return;
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8) {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp,
                              uint32_t flags) {
    if (flags & SDL_HWSURFACE)
        NDL_OpenCanvas(&width, &height);
    return SDL_CreateRGBSurface(flags, width, height, bpp, DEFAULT_RMASK,
                                DEFAULT_GMASK, DEFAULT_BMASK,
                                DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  printf("miracle to setting surface\n");
  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    printf("miracle to update rect\n");
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
}
