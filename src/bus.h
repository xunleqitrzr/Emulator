#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include "ram.h"

uint8_t bus_read(RAM* ram, uint16_t address);                 // read  through bus
void bus_write(RAM* ram, uint16_t address, uint8_t value);    // write through bus

#endif //BUS_H
