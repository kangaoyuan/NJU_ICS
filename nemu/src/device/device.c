#include <common.h>

#ifdef HAS_IOE

#include <SDL2/SDL.h>
#include <device/alarm.h>
#include <monitor/monitor.h>

void init_serial();
void init_alarm();
void init_vga();
void init_i8042();
void init_audio();
void init_timer();


static int device_update_flag = false;
static void set_device_update_flag() {
    device_update_flag = true;
}

void vga_update_screen();
void send_key(uint8_t, bool);
// device_update() function applied in cpu_exec() function,
// when every intruction be executed.
void device_update() {
    // set_device_update_flag() fucntion registered in alarm signal handler,
    // which is to control the device_update frequency.
    if (!device_update_flag) {
        return;
    }
    device_update_flag = false;
    vga_update_screen();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        // clicking the window x button to quit
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
    while (SDL_PollEvent(&event))
        ;
}

void init_device() {
    init_serial();  // serial 
    init_timer();   // timer
    init_i8042();   // keyboard
    init_vga();     // vga
    init_audio();   // audio

    add_alarm_handle(set_device_update_flag);   // register alarm handler
    init_alarm();   // alarm
}
#else

void init_device() {}

#endif	/* HAS_IOE */
