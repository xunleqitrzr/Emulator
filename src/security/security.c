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

void check_ram_write(uint16_t address, bool is_system) {
    // 1. program code is under all conditions read-only (even in system mode)
    if (address < get_usable_offset()) {
        fprintf(stderr, "Critical Error: Stack/Write collision with Program Code at 0x%04X\n", address);
        exit(1);
    }

    // 2. reserved stack area, only protected if in user mode
    if (!is_system && address >= (0x10000 - RESERVED_STACK_SIZE)) {
        fprintf(stderr, "Access Violation: User code tried to write to Stack Area at 0x%04X\n", address);
        exit(1);
    }
}