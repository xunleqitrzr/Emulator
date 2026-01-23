#ifndef SECURITY_H
#define SECURITY_H

#include "../cpu.h"
#include <stdint.h>

void security_init(CPU* cpu);

void set_usable_offset(uint16_t program_size);
uint16_t get_usable_offset();

void check_ram_write(uint16_t address, bool is_system);

#endif