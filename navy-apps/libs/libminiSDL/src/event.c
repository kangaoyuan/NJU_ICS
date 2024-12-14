#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

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
    char buf[64] = {0};

    if (NDL_PollEvent(buf, 64) == 1) {
        if (strncmp(buf, "kd", 2) == 0)
            ev->key.type = SDL_KEYDOWN;
        else
            ev->key.type = SDL_KEYUP;

        for (unsigned i = 0; i < sizeof(keyname) / sizeof(keyname[0]);
             ++i) {
            if ( !strncmp(buf + 3, keyname[i], strlen(keyname[i])) ) {
                ev->key.keysym.sym = i;
                break;
            }
        }

        //free(buf);
        return 1;
    } else {
        ev->key.type =
            SDL_USEREVENT;  // avoid too many `Redirecting file open ...`
        ev->key.keysym.sym = 0;
        //free(buf);
        return 0;
    }
}

int SDL_WaitEvent(SDL_Event* event) {
    //unsigned buf_size = 64;
    //char *buf = (char *)malloc(buf_size * sizeof(char));
    static char buf[64] = {0};

    while (NDL_PollEvent(buf, 64) == 0); // wait ...

    if (strncmp(buf, "kd", 2) == 0)
        event->key.type = SDL_KEYDOWN;
    else
        event->key.type = SDL_KEYUP;


    for (unsigned i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
        if ( strlen(buf + 3) - 1 == strlen(keyname[i]) &&
             !strncmp(buf + 3, keyname[i], strlen(keyname[i])) ) {
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
  return NULL;
}
