#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define RAM_SIZE 65536  // 64 KiB RAM

typedef struct {
    uint8_t memory[RAM_SIZE];
} RAM;

void ram_init(RAM* ram);
uint8_t ram_read(RAM* ram, uint16_t address);                   // RAM-read
void ram_write(RAM* ram, uint16_t address, uint8_t value);      // RAM-write

#endif //RAM_H
