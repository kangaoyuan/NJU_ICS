#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
    char*  name;
    size_t size;
    size_t disk_offset;
    ReadFn  read;
    WriteFn write;
    size_t open_offset;
} Finfo;

size_t events_read(void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
enum {FD_STDIN = 0, FD_STDOUT, FD_STDERR, DEV_EVENTS, PROC_DISPINFO, FD_FB};

size_t invalid_read(void* buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void* buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [DEV_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
    [PROC_DISPINFO] = {"/proc/dispinfo", 64, 0, dispinfo_read, invalid_write},
    [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
#include "files.h"
};

typedef struct {
    size_t fd;
    size_t open_offset;  // relative to disk_offset
} OFinfo;

static size_t open_file_table_index = 0;
static OFinfo open_file_table[LENGTH(file_table)];

static int get_open_file_index(int fd) {
    for(int i = 0; i < open_file_table_index; ++i){
        if(open_file_table[i].fd == fd)
            return i; 
    }
    return -1;
}

int fs_open(const char* path_name, int flags, int mode) {
    for(int i = 0; i < LENGTH(file_table); ++i) {
        if(strcmp(file_table[i].name, path_name) == 0) {
            if(i < FD_FB) {
                Log("fs_open() open char file: %s", path_name); 
                return i;
            } 
            open_file_table[open_file_table_index].fd = i;
            open_file_table[open_file_table_index++].open_offset = 0;
            return i;
        } 
    }
    panic("file %s isn't in fs, not found", path_name);
}

int fs_read(int fd, void* buf, size_t len){
    if(file_table[fd].read != NULL && fd < FD_FB)
        return file_table[fd].read(buf, 0, len);

    int target_index = get_open_file_index(fd);
    if (target_index == -1) {
        Log("fs_read file %s not opened", file_table[fd].name);
        return -1;
    }

    size_t file_size = file_table[fd].size;
    size_t disk_offset = file_table[fd].disk_offset;
    size_t open_offset = open_file_table[target_index].open_offset;
    
    if(open_offset > file_size)
        return 0;
    if(open_offset + len > file_size)
        len = file_size - open_offset;

    if(file_table[fd].read)
        len = file_table[fd].read(buf, disk_offset + open_offset, len);
    else
        len = ramdisk_read(buf, disk_offset + open_offset, len);

    open_file_table[target_index].open_offset += len;
    return len;
}

int fs_write(int fd, void* buf, size_t len){
    if(file_table[fd].write != NULL && fd < FD_FB)
        return file_table[fd].write(buf, 0, len);

    int target_index = get_open_file_index(fd);
    if (target_index == -1) {
        Log("fs_write file %s not opened", file_table[fd].name);
        return -1;
    }

    size_t file_size = file_table[fd].size;
    size_t disk_offset = file_table[fd].disk_offset;
    size_t open_offset = open_file_table[target_index].open_offset;
    
    if(open_offset > file_size)
        return 0;
    if(open_offset + len > file_size)
        len = file_size - open_offset;

    if(file_table[fd].write)
        len = file_table[fd].write(buf, disk_offset + open_offset, len);
    else
        len = ramdisk_write(buf, disk_offset + open_offset, len);

    open_file_table[target_index].open_offset += len;
    return len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
    if(fd < FD_FB) {
        Log("fs_lseek() seek char file %s", file_table[fd].name); 
        return 0;
    }

    int target_index = get_open_file_index(fd);
    if(target_index == -1){
        Log("file %s not fs_open before fs_lseek", file_table[fd].name); 
        return -1;
    }

    size_t new_offset = -1;
    size_t file_size = file_table[fd].size;
    size_t open_offset = open_file_table[target_index].open_offset;
    switch (whence) {
    case SEEK_SET:
        if (offset > file_size)
            new_offset = file_size;
        else
            new_offset = offset;
        break;
    case SEEK_CUR:
        if (open_offset + offset > file_size)
            new_offset = file_size;
        else
            new_offset = offset + open_offset;
        break;
    case SEEK_END:
        if (file_size + offset > file_size)
            new_offset = file_size;
        else
            new_offset = offset + file_size;
        break;
    default:
        panic("Unknown whence in fs_lseek() %d", whence);
    }

    open_file_table[target_index].open_offset = new_offset;
    return new_offset;
}

int fs_close(int fd){
    if(fd <= FD_FB) {
        Log("fs_close() close char file %s", file_table[fd].name); 
        return 0;
    }

    int target_index = get_open_file_index(fd);
    if(target_index == -1){
        Log("file %s not fs_open before fs_close", file_table[fd].name); 
        return -1;
    }

    for (int i = target_index; i < open_file_table_index - 1; ++i) {
        open_file_table[i] = open_file_table[i + 1];
    }
    open_file_table_index--;
    assert(open_file_table_index >= 0);
    return 0;
}

void init_fs() {
    // TODO: initialize the size of /dev/fb
    AM_GPU_CONFIG_T conf = io_read(AM_GPU_CONFIG);
    int             width = conf.width;
    int             height = conf.height;
    file_table[FD_FB].size = width * height * 4;
}
