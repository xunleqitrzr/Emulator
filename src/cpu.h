#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "ram.h"

// flags
#define FLAG_ZERO       0x01        // bit 0 --> 0001
#define FLAG_CARRY      0x02        // bit 1 --> 0010
#define FLAG_SIGN       0x04        // bit 2 --> 0100
#define FLAG_OVERFLOW   0x08        // bit 3 --> 1000

typedef struct {
    uint8_t registers[4];   // general purpose registers
    uint16_t PC;            // program counter
    uint16_t SP;            // stack pointer
    uint8_t FLAGS;          // flags register
    bool halted;            // stop execution flag
} CPU;

typedef enum {
    A = 0x00,          // A register
    B = 0x01,          // B register
    C = 0x02,          // C register
    D = 0x03,          // D register
} Register;

typedef enum {
    NOP = 0x00,         // no operation
    LDA = 0x01,         // load A from memory (accumulator)
    LDB = 0x02,         // load B from memory
    LDI = 0x03,         // load given value immediately into A
    INC = 0x04,         // increment A
    DEC = 0x05,         // decrement A
    ADD = 0x06,         // add <register2> to <register1>: ADD A, B
    SUB = 0x07,         // subtract B from A: SUB A, B
    MUL = 0x08,         // multiply A with B: MUL A, B
    STA = 0x09,         // store A to memory
    STB = 0x0A,         // store B to memory
    MOV = 0x0B,         // move to registers: MOV A, B
    CMP = 0x0C,         // compare A to B
    JMP = 0x0D,         // jump to address
    JZ  = 0x0E,         // jump if zero flag is set
    JNZ = 0x0F,         // jump if zero flag is not set
    JC  = 0x10,         // jump if carry flag is set
    JNC = 0x11,         // jump if carry flag is not set
    JE  = 0x12,         // jump if equal
    JNE = 0x13,         // jump if not equal
    JL  = 0x14,         // jump if less (signed)
    JG  = 0x15,         // jump if greater (signed)
    JB  = 0x16,         // jump if below (unsigned)
    JA  = 0x17,         // jump if above (unsigned)
    AND = 0x18,         // bitwise AND: AND <dest>, <src>
    OR  = 0x19,         // bitwise OR: OR <dest>, <src>
    XOR = 0x1A,         // bitwise XOR: XOR <dest>, <src>
    NOT = 0x1B,         // bitwise NOT: NOT <src>
    PUSH = 0x1C,        // push onto stack
    POP = 0x1D,         // pop off stack
    CALL = 0x1E,        // call <addr>
    RET = 0x1F,         // return
    HLT = 0xFF          // halt CPU
} Instruction;

// CPU
void cpu_reset(CPU *cpu);
void cpu_step(CPU* cpu, RAM* ram);

// FLAG LOGIC
void set_flag(uint8_t* flags, uint8_t mask);
void clear_flag(uint8_t* flags, uint8_t mask);
bool is_flag_set(uint8_t flags, uint8_t mask);

// FLAG HELPER FUNCTIONS
#define SET_FLAG_IF(cpu, condition, flag) \
    do { \
        if (condition) set_flag(&cpu->FLAGS, flag); \
        else clear_flag(&cpu->FLAGS, flag); \
    } while (0)

void set_flags_add(CPU* cpu, uint8_t reg1, uint8_t reg2, uint16_t result);
void set_flags_sub(CPU* cpu, uint8_t reg1, uint8_t reg2, uint16_t result);
void set_flags_inc(CPU* cpu, uint8_t original, uint16_t result);
void set_flags_dec(CPU* cpu, uint8_t original, uint16_t result);
void set_flags_bitwise_ops(CPU* cpu, uint8_t result);
void set_flags_mul(CPU* cpu, uint16_t result);

// REGISTER BOUNDS CHECK
bool register_out_of_bounds(CPU* cpu, uint8_t registers);

// MISC
size_t get_number_of_registers(CPU* cpu);

// DEBUG
void print_state(CPU* cpu);
void print_flags(uint8_t flags);

#endif //CPU_H
