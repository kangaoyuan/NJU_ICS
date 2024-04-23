#include <common.h>
#include "syscall.h"

size_t fs_write(int fd,const void *buf,size_t len);

void do_syscall(Context* c) {
    uintptr_t a[4];
    a[0] = c->GPR1;

    switch (a[0]) {
    case SYS_exit:
        halt(c->GPR2);
        break;
    case SYS_yield:
        yield();
        c->GPRx = 0;
        break;
    case SYS_write: {
        int    fd = (int)c->GPR2;
        char*  buf = (char*)c->GPR3;
        size_t len = (size_t)c->GPR4;  // if (len!=14) assert(0);
        if (fd != 1 && fd != 2)
            c->GPRx = -1;//fs_write(fd, buf, len);  // c->GPRx=-1;
        else {
            for (int i = 0; i < len; ++i, ++buf)
                putch(*buf);
            c->GPRx = len;
        }
        break;
    }
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
}
