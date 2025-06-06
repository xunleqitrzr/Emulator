#ifndef BUS_H
#define BUS_H

#include <stdint.h>

uint8_t bus_read(uint16_t address);
void bus_write(uint16_t address, uint8_t value);

#endif //BUS_H
