#ifndef __DEVICE_MAP_H__
#define __DEVICE_MAP_H__

#include <monitor/difftest.h>

// IOMap struct mapped space allocation.
uint8_t* new_space(int size);
typedef void(*io_callback_t)(uint32_t, int, bool);

typedef struct {
    char* name;
    // [low, high] is the source address.
    paddr_t       low;
    paddr_t       high;
    // space is the destination mapped address.
    uint8_t*      space;
    io_callback_t callback;
} IOMap;

// port-mapped-I/O, memory-mapped-I/O
void add_pio_map(char *name, ioaddr_t addr, uint8_t *space, int len, io_callback_t callback);
void add_mmio_map(char *name, paddr_t addr, uint8_t* space, int len, io_callback_t callback);

word_t map_read(paddr_t addr, int len, IOMap *map);
void map_write(paddr_t addr, word_t data, int len, IOMap *map);



static inline bool map_inside(IOMap* map, paddr_t addr) {
    return (map->low <= addr && addr <= map->high);
}

static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
    for (int i = 0; i < size; i++) {
        if (map_inside(maps + i, addr)) {
            difftest_skip_ref();
            return i;
        }
    }
    return -1;
}

#endif
