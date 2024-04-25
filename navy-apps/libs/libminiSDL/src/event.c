#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 64
#define keyname(k) #k,
#define NR_KEYS ( sizeof(keyname)/sizeof(keyname[0]) )

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
    assert(0);
    return 0;
}

int SDL_PollEvent(SDL_Event* ev) {
    //unsigned buf_size = 64;
    //char*    buf = (char*)malloc(buf_size * sizeof(char));
    char buf[BUF_SIZE] = {0};

    if (NDL_PollEvent(buf, sizeof(buf)) == 1) {
        if (strncmp(buf, "kd", 2) == 0)
            ev->key.type = SDL_KEYDOWN;
        else
            ev->key.type = SDL_KEYUP;

        for (unsigned i = 0; NR_KEYS; ++i) {
            if ( strlen(keyname[i]) == strlen(buf) - 4 &&
                !strncmp(buf + 3, keyname[i], strlen(buf) - 4)) {
                ev->key.keysym.sym = i;
                break;
            }
        }

        //free(buf);
        return 1;
    } else {
        ev->key.type = SDL_USEREVENT;
        ev->key.keysym.sym = SDLK_NONE;
        //free(buf);
        return 0;
    }
}

int SDL_WaitEvent(SDL_Event* event) {
    //unsigned buf_size = 64;
    //char *buf = (char *)malloc(buf_size * sizeof(char));
    static char buf[64] = {0};

    while (NDL_PollEvent(buf, 64) == 0); // wait ...

    printf("== The key is %s, we need to see. ===\n", buf );

    if (strncmp(buf, "kd", 2) == 0)
        event->key.type = SDL_KEYDOWN;
    else
        event->key.type = SDL_KEYUP;


    for (unsigned i = 0; i < NR_KEYS; ++i) {
        if ( strlen(keyname[i]) == strlen(buf) - 4 &&
            !strncmp(buf + 3, keyname[i], strlen(buf) - 4)) {
            event->key.keysym.sym = i;
            break;
        }
    }

    //free(buf);
    return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
    assert(0);
    return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  assert(0);
  return NULL;
}
