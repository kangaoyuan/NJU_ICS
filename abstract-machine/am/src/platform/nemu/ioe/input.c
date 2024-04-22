#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T* kbd) {
    uint32_t code_key = inl(KBD_ADDR);
    if (code_key == AM_KEY_NONE) {
        kbd->keydown = 0;
        kbd->keycode = AM_KEY_NONE;
    } else {
        if (code_key & KEYDOWN_MASK)
            kbd->keydown = 1;
        else
            kbd->keydown = 0;
        kbd->keycode = (code_key & ~KEYDOWN_MASK);
    }
}
