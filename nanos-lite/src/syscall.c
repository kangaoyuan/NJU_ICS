#include <common.h>
#include "syscall.h"

int sys_yield() {
    yield();
    return 0;
}

int sys_brk(void *addr) {
    return 0;
}

void sys_exit(int status) {
    halt(status);
}

#define time_t uint64_t
typedef uint64_t susconds_t;

struct timeval{
    time_t tv_sec;
    susconds_t tv_usec;
};

struct timezone{
    int tz_minuteswest;
    int tz_dsttime;
};

int sys_gettimeofday(struct timeval* tv, struct timezone* tz){
    uint64_t us = io_read(AM_TIMER_UPTIME).us;
    tv->tv_sec = us / 1000000;
    tv->tv_usec = us - us / 1000000 * 1000000;
    return 0;
}

int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd,const void *buf,size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);

// be called from do_event(), from __am_irq_handle(), from _[syscall_name](), for [syscall_name]().
void do_syscall(Context* c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    switch (a[0]) {
    case SYS_exit:
        sys_exit(c->GPR2);
        break;
    case SYS_yield:
        c->GPRx = sys_yield();
        break;
    case SYS_brk:
        c->GPRx = sys_brk((void *)c->GPR2);
        Log("sys_brk(%p, %d, %d) = %d", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    case SYS_open:
        c->GPRx = fs_open((const char *)c->GPR2, c->GPR3, c->GPR4);
        Log("fs_open(%s, %d, %d) = %d",(const char *)c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    case SYS_read:
        c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, (size_t)c->GPR4);
        Log("fs_read(%d, %p, %d) = %d", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    case SYS_close:
        c->GPRx = fs_close(c->GPR2);
        Log("fs_close(%d, %d, %d) = %d", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    case SYS_write:
        c->GPRx = fs_write(c->GPR2, (void *)c->GPR3, (size_t)c->GPR4);
        Log("fs_write(%d, %p, %d) = %d", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    case SYS_lseek:
        c->GPRx = fs_lseek(c->GPR2, (size_t)c->GPR3, c->GPR4);
        Log("fs_lseek(%d, %d, %d) = %d", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    case SYS_gettimeofday:
        c->GPRx = sys_gettimeofday((struct timeval*)c->GPR2, (struct timezone*)c->GPR3);
        Log("sys_gettimeofday(%p, %p, %d) = %d", c->GPR2, c->GPR3, c->GPR4, c->GPRx);
        break;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
}
