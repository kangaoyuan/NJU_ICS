#include <proc.h>
#include <common.h>
#include "syscall.h"
#include <sys/time.h>

int    fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int    fs_close(int fd);
void   switch_boot_pcb();
int    mm_brk(uintptr_t brk);
void   naive_uload(PCB *pcb, const char *filename);
void   context_uload(PCB *pcb, const char *file_name, char * const *argv, char * const envp[]);
int    sys_gettimeofday(struct timeval* tv, struct timezone* tz);
int    sys_execve(const char* filename,char * const argv[],char* const envp[]);

void do_syscall(Context* c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    switch (a[0]) {
    case SYS_exit:
        halt(a[1]);
        //sys_execve("/bin/menu", NULL, NULL);
        //sys_execve("/bin/nterm", NULL, NULL);
        break;
    case SYS_yield:
        yield();
        c->GPRx = 0;
        break;
    case SYS_write: {
        int    fd = (int)a[1];
        char*  buf = (char*)a[2];
        size_t len = (size_t)a[3];
        c->GPRx = fs_write(fd, buf, len);
        break;
    }
    case SYS_brk:
        //c->GPRx = 0;
        c->GPRx = mm_brk(a[1]);
        break;
    case SYS_open:
        // The same argument meaning to fs_ function. 
        c->GPRx = fs_open((void*)a[1], a[2], a[3]);
        break;
    case SYS_read:
        c->GPRx = fs_read(a[1], (void*)a[2], a[3]);
        break;
    case SYS_lseek:
        c->GPRx = fs_lseek(a[1], a[2], a[3]);
        break;
    case SYS_close:
        c->GPRx = fs_close(a[1]);
        break;
    case SYS_gettimeofday:
        c->GPRx = sys_gettimeofday((struct timeval *)a[1],(struct timezone *)a[2]);
        break;
    case SYS_execve:
        c->GPRx = sys_execve((const char*)a[1], (char* const*)a[2], (char* const*)a[3]);
        break;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
}

int sys_execve(const char* file_name,char* const argv[],char* const envp[]){
    //naive_uload(NULL, file_name);
    if (fs_open(file_name, 0, 0) == -1)
        return -2;

    context_uload(current, file_name, argv, envp);
    switch_boot_pcb();
    yield();
    return -1;
}
