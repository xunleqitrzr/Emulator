#ifndef ROM_H
#define ROM_H

#include <stdint.h>
#include "ram.h"

/*
typedef struct {
    uint8_t memory[65536];      // 64 KB program ROM
} ROM;
uint8_t rom_read(ROM* rom, uint16_t address);   // read ROM (program)
*/

void rom_load(RAM* ram, const uint8_t* program, uint16_t size);

#endif //ROM_H
