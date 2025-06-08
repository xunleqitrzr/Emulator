#include "ram.h"

#include <stdbool.h>

void ram_init(RAM* ram) {
    for (int i = 0; i < RAM_SIZE; i++) {
        ram->memory[i] = 0;
    }
}

uint8_t ram_read(RAM* ram, uint16_t address) {
    return ram->memory[address];
}

void ram_write(RAM* ram, uint16_t address, uint8_t value) {
    ram->memory[address] = value;
}