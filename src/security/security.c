#include "security.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

static uint16_t usable_offset = 0;
static uint16_t stack_pointer = 0;

void security_init(CPU* cpu) {
    stack_pointer = cpu->SP;
}

void set_usable_offset(uint16_t program_size) {
    usable_offset = program_size;
}

uint16_t get_usable_offset() {
    return usable_offset;
}

uint16_t get_sp() {
    return stack_pointer;
}

void check_ram_write(uint16_t address) {
    if (address < get_usable_offset()) {
        fprintf(stderr, "RAM write to protected address %" PRIu16 " (0x%08x) denied\t(PROGRAM OVERWRITE)\n", address, address);
        exit(1);
    }
    if (address >= get_sp()) {
        fprintf(stderr, "RAM write to protected address %" PRIu16 " (0x%08x) denied\t(STACK OVERWRITE)\n", address, address);
        exit(1);
    }
}