#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "ram.h"

#define FLAG_ZERO  0x01     // bit 0
#define FLAG_CARRY 0x02     // bit 1

typedef struct {
    uint8_t A, B;       // general purpose registers
    uint16_t PC;        // program counter
    uint16_t SP;        // stack pointer
    uint8_t FLAGS;      // flags register
    bool halted;        // stop execution flag
} CPU;

typedef enum {
    REG_A = 0,          // A register
    REG_B = 1           // B register
} Register;

typedef enum {
    NOP = 0x00,         // no operation
    LDA = 0x01,         // load A from memory (accumulator)
    LDB = 0x02,         // load B from memory
    LDI = 0x03,         // load given value immediately into A
    INC = 0x04,         // increment A
    DEC = 0x05,         // decrement A
    ADD = 0x06,         // add B to A
    SUB = 0x07,         // subtract B from A
    MUL = 0x08,         // multiply A with B
    STA = 0x09,         // store A to memory
    STB = 0x0A,         // store B to memory
    MOV = 0x0B,         // move register into A (for now, there is only one other register than A => B)
    CMP = 0x0C,         // compare A to B
    JMP = 0x0D,         // jump to address
    JZ  = 0x0E,         // jump if zero flag is set
    JNZ = 0x0F,         // jump if zero flag is not set
    JC  = 0x10,         // jump if carry flag is set
    JNC = 0x11,         // jump if carry flag is not set
    JE  = 0x12,         // jump if equal
    JNE = 0x13,         // jump if not equal
    HLT = 0xFF          // halt CPU
} Instruction;

void cpu_reset(CPU *cpu);
void cpu_step(CPU* cpu, RAM* ram);

void set_flag(uint8_t* flags, uint8_t mask);
void clear_flag(uint8_t* flags, uint8_t mask);
bool is_flag_set(uint8_t flags, uint8_t mask);
void update_flags(CPU* cpu, uint16_t result);

#endif //CPU_H
