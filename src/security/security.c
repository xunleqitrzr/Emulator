#include "security.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define RESERVED_STACK_SIZE 0x100

static uint16_t usable_offset = 0;

void security_init(CPU* cpu) {
    (void) cpu;
}

void set_usable_offset(uint16_t program_size) {
    usable_offset = program_size;
}

uint16_t get_usable_offset() {
    return usable_offset;
}

void check_ram_write(uint16_t address) {
    // 1.program protection
    if (address < get_usable_offset()) {
        fprintf(stderr, "Write denied: Address 0x%04X is inside Program Code.\n", address);
        exit(1);
    }
    if (address >= (0x10000 - RESERVED_STACK_SIZE)) {
        fprintf(stderr, "Write denied: Address 0x%04X is inside Reserved Stack Area.\n", address);
        exit(1);
    }
}