#include <common.h>

#ifdef HAS_IOE

#include <SDL2/SDL.h>
#include <device/alarm.h>
#include <monitor/monitor.h>

void init_vga();
void init_timer();
void init_alarm();
void init_i8042();
void init_audio();
void init_serial();

void vga_update_screen();
void send_key(uint8_t, bool);

static int device_update_flag = false;

static void set_device_update_flag() {
    device_update_flag = true;
}

void device_update() {
    // The true val is updated through the timer callback.
    if (!device_update_flag) {
        return;
    }
    device_update_flag = false;

    vga_update_screen();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        // Click the x button to quit
        case SDL_QUIT:
            nemu_state.state = NEMU_QUIT;
            break;
        // If a key was pressed
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            uint8_t k = event.key.keysym.scancode;
            bool    is_keydown = (event.key.type == SDL_KEYDOWN);
            send_key(k, is_keydown);
            break;
        }
        default:
            break;
        }
    }
}

void sdl_clear_event_queue() {
    SDL_Event event;
    // Throw away the remain keyboard event.
    while (SDL_PollEvent(&event))
        ;
}

void init_device() {
    init_serial();
    init_timer();
    init_vga();
    init_i8042();
    init_audio();

    add_alarm_handle(set_device_update_flag);
    init_alarm();
}
#else

void init_device() {
}

#endif	/* HAS_IOE */
