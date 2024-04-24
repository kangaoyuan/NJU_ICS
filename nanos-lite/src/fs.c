#include <fs.h>

#define FILE_NUM sizeof(file_table)/sizeof(Finfo)

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn)(void* buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void* buf, size_t offset, size_t len);

typedef struct {
    char*   name;
    size_t  size;
    size_t  disk_offset;
    ReadFn  read;
    WriteFn write;
    size_t  open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS};

size_t invalid_read(void* buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void* buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char* pathname, int flags, int mode) {
    int i, fd = -1;
    printf("search file_name = %s\n", pathname);
    for (i = 0; i < FILE_NUM; ++i) {
        printf("[%d] file_name = %s\n", i, file_table[i].name);
        if (!strcmp(pathname, file_table[i].name)) {
            fd = i;
            file_table[i].open_offset = 0;
            break;
        }
    }
    assert(i == FILE_NUM);
    return fd;
    //return i == FILE_NUM ? -1 : fd;
}

size_t fs_read(int fd, void* buf, size_t len) {
    size_t ret = -1, true_len = -1;

    if (file_table[fd].read != NULL) {
        ret = file_table[fd].read(buf, file_table[fd].open_offset, len);
    } else {
        if (fd == FD_EVENTS ||
            file_table[fd].open_offset + len < file_table[fd].size) {
            true_len = len;
        } else {
            true_len = file_table[fd].size - file_table[fd].open_offset;
        }

        ret = ramdisk_read(
            buf, file_table[fd].disk_offset + file_table[fd].open_offset,
            true_len);
    }
    file_table[fd].open_offset += ret;
    assert(file_table[fd].open_offset >= 0 &&
           file_table[fd].open_offset <= file_table[fd].size);
    return ret;
}

size_t fs_write(int fd, const void* buf, size_t len) {
    size_t ret = -1, true_len = -1;

    if (file_table[fd].write != NULL) {
        ret = file_table[fd].write(buf, file_table[fd].open_offset, len);
    } else {
        if (fd == FD_EVENTS ||
            file_table[fd].open_offset + len < file_table[fd].size) {
            true_len = len;
        } else {
            true_len = file_table[fd].size - file_table[fd].open_offset;
        }
        ret = ramdisk_write(
            buf, file_table[fd].disk_offset + file_table[fd].open_offset,
            true_len);
    }
    file_table[fd].open_offset += ret;
    assert(file_table[fd].open_offset >= 0 &&
           file_table[fd].open_offset <= file_table[fd].size);
    return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
    switch (whence) {
    case SEEK_SET:
        file_table[fd].open_offset = offset;
        break;
    case SEEK_CUR:
        file_table[fd].open_offset += offset;
        break;
    case SEEK_END:
        file_table[fd].open_offset = file_table[fd].size + offset;
        break;
    default:
        return -1;
    }
    assert(file_table[fd].open_offset >= 0 &&
           file_table[fd].open_offset <= file_table[fd].size);
    return file_table[fd].open_offset;
}

int fs_close(int fd) {
    return 0;
}
