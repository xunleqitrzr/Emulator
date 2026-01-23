#include <stdlib.h>
#include "bus.h"
#include "gpu.h"
#include "security/security.h"

static uint8_t sys_clock_counter = 0;   // internal ticker

void bus_tick() {
    sys_clock_counter++;
}

uint8_t bus_read(RAM* ram, uint16_t address) {
    if (address == MMIO_RNG) {
        return rand() % 255;
    }
    if (address == MMIO_TIMER) {
        return sys_clock_counter;
    }

    return ram_read(ram, address);
}

void bus_write(RAM* ram, uint16_t address, uint8_t value) {
    check_ram_write(address, false);        // false = user mode
    ram_write(ram, address, value);

    if (address >= MMIO_GPU_START && address <= MMIO_GPU_END) {
        gpu_dirty = true;
    }
}

void bus_write_system(RAM* ram, uint16_t address, uint8_t value) {
    check_ram_write(address, true);         // true = system mode
    ram_write(ram, address, value);

    if (address >= MMIO_GPU_START && address <= MMIO_GPU_END) {
        gpu_dirty = true;
    }
}

