#include "rom.h"

void rom_load(RAM* ram, const uint8_t* program, uint16_t size) {
    for (uint16_t i = 0; i < size; ++i) {
        ram_write(ram, i, program[i]);
    }
}
