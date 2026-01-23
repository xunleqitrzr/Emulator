#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include <stdbool.h>
#include "ram.h"

// -- MEMORY MAP --
// 0x0000 - 0x3FFF: General Purpose RAM (16KB for program + data)
// 0x4000 - 0x40FF: Video Memory (MMIO)
// 0x4100 - 0xFFFF: General Purpose RAM / Stack
#define MMIO_GPU_START 0x4000
#define MMIO_GPU_END   0x40FF
#define MMIO_TIMER     0xF000   // advance time
#define MMIO_RNG       0x9000   // reading from here returns a random byte

extern bool gpu_dirty;

void bus_tick(void);                                                // advance time
uint8_t bus_read(RAM* ram, uint16_t address);                       // read  through bus
void bus_write(RAM* ram, uint16_t address, uint8_t value);          // write through bus
void bus_write_system(RAM* ram, uint16_t address, uint8_t value);   // privileged write through bus

#endif //BUS_H
