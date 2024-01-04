#include <device/map.h>

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

// serial is cha stream device, which doesn't support offset.
#define CH_OFFSET 0
#define SERIAL_PORT 0x3F8
#define SERIAL_MMIO 0xa10003F8
static uint8_t *serial_base = NULL;

static void serial_io_handler(uint32_t offset, int len, bool is_write) {
    assert(len == 1);
    switch (offset) {
    case CH_OFFSET:
        // only valid when map_write() be callback.
        if (is_write)
            /* We bind the serial port with the host unbuffered stderr. */
            putc(serial_base[0], stderr);
        else
            panic("I/O serial device, which do not support read");
        break;
    default:
        panic("I/O serial device, which do not support offset = %d", offset);
    }
}

void init_serial() {
    serial_base = new_space(8);
    add_pio_map("serial", SERIAL_PORT, serial_base, 8, serial_io_handler);
    add_mmio_map("serial", SERIAL_MMIO, serial_base, 8, serial_io_handler);
}
