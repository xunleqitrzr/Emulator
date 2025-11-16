#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define RAM_SIZE 65536                  // 64 KiB RAM
#define MMIO_VISUAL_BEGIN 0x4000        // 16384
#define MMIO_VISUAL_END 0x40FF          // 16639    => 255 bytes of video buffer

typedef struct {
    uint8_t memory[RAM_SIZE];
} RAM;

void ram_init(RAM* ram);                                            // initialize RAM
uint8_t ram_read(RAM* ram, uint16_t address);                       // RAM-read
void ram_write(RAM* ram, uint16_t address, uint8_t value);          // RAM-write
void ram_write_program(RAM* ram, uint16_t address, uint8_t value);  // write the program once, no security checking

#endif //RAM_H
