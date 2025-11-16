#include "ram.h"

#include "security/security.h"
#include <stdio.h>

void ram_init(RAM* ram) {
    for (int i = 0; i < RAM_SIZE; i++) {
        ram->memory[i] = 0;
    }
}

uint8_t ram_read(RAM* ram, uint16_t address) {
    return ram->memory[address];
}

void ram_write(RAM* ram, uint16_t address, uint8_t value) {
    check_ram_write(address);
    if ((address >= MMIO_VISUAL_BEGIN) && (address <= MMIO_VISUAL_END)) {
        // memory mapped i/o: print character to console
        printf("%c", value);
        fflush(stdout);
    }
    ram->memory[address] = value;
}

void ram_write_program(RAM* ram, uint16_t address, uint8_t value) {     // no security check; only called once at emulator start up
    ram->memory[address] = value;
}