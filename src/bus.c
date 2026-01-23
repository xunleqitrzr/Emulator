#include "bus.h"
#include "gpu.h"

uint8_t bus_read(RAM* ram, uint16_t address) {
    return ram_read(ram, address);
}

void bus_write(RAM* ram, uint16_t address, uint8_t value) {
    ram_write(ram, address, value);


    if (address >= MMIO_VISUAL_BEGIN && address <= MMIO_VISUAL_END) {
        gpu_dirty = true;
    }
}

