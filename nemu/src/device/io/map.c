#include <isa.h>
#include <device/map.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

#define IO_SPACE_MAX (2 * 1024 * 1024)

static uint8_t io_space[IO_SPACE_MAX] PG_ALIGN = {};
static uint8_t *p_space = io_space;

uint8_t* new_space(int size) {
    uint8_t* p_alloc = p_space;
    // upper to align
    size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
    p_space += size;
    assert(p_space - io_space < IO_SPACE_MAX);
    return p_alloc;
}

static inline void invoke_callback(io_callback_t c,
                                   paddr_t offset, int len, bool is_write){
    if (c != NULL)
        c(offset, len, is_write);
}

static inline void check_bound(IOMap* map, paddr_t addr) {
    Assert(map != NULL && map_inside(map, addr),
           "address (0x%08x) is out of bound {%s} [0x%08x, 0x%08x] at pc "
           "= " FMT_WORD,
           addr, (map ? map->name : "???"), (map ? map->low : 0),
           (map ? map->high : 0), cpu.pc);
}

// map_read or map_write is the paddr access of I/O address.
// Attention: map_read or map_write once access length range is [1, 8].
word_t map_read(paddr_t addr, int len, IOMap* map) {
    check_bound(map, addr);
    assert(len >= 1 && len <= 8);
    paddr_t offset = addr - map->low;

    invoke_callback(map->callback, offset, len, false);
    word_t data = *(word_t*)(map->space + offset) & (~0Lu >> ((8 - len) << 3));
    return data;
}

void map_write(paddr_t addr, word_t data, int len, IOMap* map) {
    check_bound(map, addr);
    assert(len >= 1 && len <= 8);
    paddr_t offset = addr - map->low;

    memcpy(map->space + offset, &data, len);
    invoke_callback(map->callback, offset, len, true);
}
