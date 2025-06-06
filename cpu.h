#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "ram.h"

typedef struct {
    uint8_t A, B;       // general purpose registers
    uint16_t PC;        // program counter
    uint16_t SP;        // stack pointer
    uint16_t FLAGS;     // flags register
    bool halted;        // stop execution flag
} CPU;

typedef enum {
    NOP = 0x00,         // no operation
    LDA = 0x01,         // load A from memory (accumulator)
    LDB = 0x02,         // load B from memory
    ADD = 0x03,         // add B to A
    STA = 0x04,         // store A to memory
    STB = 0x05,         // store B to memory
    JMP = 0x06,         // jump to address
    HLT = 0xFF          // halt CPU
} Instruction;

void cpu_reset(CPU *cpu);
void cpu_step(CPU* cpu, RAM* ram);

#endif //CPU_H
