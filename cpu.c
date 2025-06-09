#include "cpu.h"

#include <stdlib.h>

// FLAG LOCIG
void set_flag(uint8_t* flags, uint8_t mask) {
    *flags |= mask;
}

void clear_flag(uint8_t* flags, uint8_t mask) {
    *flags &= ~mask;
}

bool is_flag_set(uint8_t flags, uint8_t mask) {
    return flags & mask;
}

void update_flags(CPU* cpu, uint16_t result) {
    if (result > 0xFF) set_flag(&cpu->FLAGS, FLAG_CARRY);
    else clear_flag(&cpu->FLAGS, FLAG_CARRY);
    if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
    else clear_flag(&cpu->FLAGS, FLAG_ZERO);
}

// FLAG HELPER FUNCTIONS
void set_flags_add(CPU* cpu, uint8_t reg1, uint8_t reg2, uint8_t result) {
    SET_FLAG_IF(result == 0, FLAG_ZERO);
    SET_FLAG_IF((result & 0x80), FLAG_SIGN);
    SET_FLAG_IF((uint16_t)reg1 + (uint16_t)reg2 > 0xFF, FLAG_CARRY);
    SET_FLAG_IF(((reg1 ^ result) & (reg2 ^ result)) & 0x80, FLAG_OVERFLOW);
}

void set_flags_sub(CPU* cpu, uint8_t reg1, uint8_t reg2, uint8_t result) {
    SET_FLAG_IF(result == 0, FLAG_ZERO);
    SET_FLAG_IF((result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(reg1 < reg2, FLAG_CARRY);
    SET_FLAG_IF(((reg1 ^ reg2) & (reg1 ^ result)) & 0x80, FLAG_OVERFLOW);
}

void set_flags_inc(CPU* cpu, uint8_t original, uint8_t result) {
    SET_FLAG_IF(result == 0, FLAG_ZERO);
    SET_FLAG_IF((result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(original == 0x7F, FLAG_OVERFLOW);
    // CF unaffected
}

void set_flags_dec(CPU* cpu, uint8_t original, uint8_t result) {
    SET_FLAG_IF(result == 0, FLAG_ZERO);
    SET_FLAG_IF((result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(original == 0x80, FLAG_OVERFLOW);
    // CF unaffected
}

void set_flags_bitwise_ops(CPU* cpu, uint8_t result) {
    SET_FLAG_IF(result == 0, FLAG_ZERO);
    SET_FLAG_IF((result & 0x80), FLAG_SIGN);
    // CF and OF cleared on these instructions on many CPUs
    clear_flag(&cpu->FLAGS, FLAG_CARRY);
    clear_flag(&cpu->FLAGS, FLAG_OVERFLOW);
}

void set_flags_mul(CPU* cpu, uint16_t result) {
    uint8_t low  = result & 0xFF;
    uint8_t high = (result >> 8) & 0xFF;
    SET_FLAG_IF(low == 0, FLAG_ZERO);
    SET_FLAG_IF(low & 0x80, FLAG_SIGN);
    // carry if high byte != 0
    SET_FLAG_IF(high != 0, FLAG_CARRY);
    clear_flag(&cpu->FLAGS, FLAG_OVERFLOW);
}

// REGISTER BOUNDS CHECK
bool register_out_of_bounds(CPU* cpu, uint8_t registers) {
    size_t array_elems = sizeof(cpu->registers) / sizeof(cpu->registers[0]);

    if (registers >= array_elems) {
        return true;
    }

    return false;
}

// MISC
size_t get_number_of_registers(CPU* cpu) {
    const size_t reg_num = sizeof(cpu->registers) / sizeof(cpu->registers[0]);
    return reg_num;
}

//CPU
void cpu_reset(CPU *cpu) {
    for (size_t i = 0; i < get_number_of_registers(cpu); i++) {
        cpu->registers[i] = 0;
    }
    cpu->PC = 0x0000;
    cpu->SP = RAM_SIZE - 1;     // 0x100 -> but stack grows downwards
    cpu->FLAGS = 0;
    cpu->halted = false;
}

void cpu_step(CPU* cpu, RAM* ram) {
    if (cpu->halted) return;

    uint8_t opcode = ram_read(ram, cpu->PC++);

    switch (opcode) {
        case NOP:
            break;

        case LDA: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            cpu->registers[A] = ram_read(ram, addr);

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case LDB: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            cpu->registers[B] = ram_read(ram, addr);
            break;
        }

        case LDI: {
            uint8_t immediate_value = ram_read(ram, cpu->PC++);
            cpu->registers[A] = immediate_value;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case INC: {      // not updating the carry flag on purpose
            uint16_t result = cpu->registers[A] + 1;

            // Zero
            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);

            // Sign
            if (result & 0x80) set_flag(&cpu->FLAGS, FLAG_SIGN);
            else clear_flag(&cpu->FLAGS, FLAG_SIGN);

            // Overflow
            if ((cpu->registers[A] == 0x7F)) set_flag(&cpu->FLAGS, FLAG_OVERFLOW);
            else clear_flag(&cpu->FLAGS, FLAG_OVERFLOW);

            cpu->registers[A] = result;
            break;
        }

        case DEC: {      // not updating the carry flag on purpose
            uint16_t result = cpu->registers[A] - 1;
            cpu->registers[A] = result;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case ADD: {     // ADD C, B
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint8_t result = a + b;

            set_flags_add(cpu, a, b, result);
            cpu->registers[reg_to] = result;
            break;
        }

        case SUB: {     // SUB C, B
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint8_t result = a - b;

            set_flags_sub(cpu, a, b, result);
            cpu->registers[reg_to] = result;
            break;
        }

        case MUL: {     // MUL D, B
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint16_t result = a * b;

            set_flags_mul(cpu, result);
            cpu->registers[reg_to] = (uint8_t) (result & 0xFF); // store low
            break;
        }

        case STA: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            ram_write(ram, addr, cpu->registers[A]);
            break;
        }

        case STB: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            ram_write(ram, addr, cpu->registers[B]);
            break;
        }

        case MOV: {     // move B register into A: MOV A, B
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            cpu->registers[reg_to] = cpu->registers[reg_from];

            break;
        }

        case CMP: {     // CMP B, D
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint8_t result = a - b;

            set_flags_sub(cpu, a, b, result);
            break;
        }

        case JMP: {     // unconditional jump
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            cpu->PC = addr;
            break;
        }

        case JZ: {      // jump if zero
            if (is_flag_set(cpu->FLAGS, FLAG_ZERO)) {
                uint16_t addr = ram_read(ram, cpu->PC++) << 8;
                addr |= ram_read(ram, cpu->PC++);
                cpu->PC = addr;
                break;
            }
            else {
                cpu->PC += 2;
                break;
            }
        }

        case JNZ: {     // jump if not zero
            if (!is_flag_set(cpu->FLAGS, FLAG_ZERO)) {
                uint16_t addr = ram_read(ram, cpu->PC++) << 8;
                addr |= ram_read(ram, cpu->PC++);
                cpu->PC = addr;
                break;
            }
            else {
                cpu->PC += 2;
                break;
            }
        }

        case JC: {      // jump if carry flag is set
            if (is_flag_set(cpu->FLAGS, FLAG_CARRY)) {
                uint16_t addr = ram_read(ram, cpu->PC++) << 8;
                addr |= ram_read(ram, cpu->PC++);
                cpu->PC = addr;
                break;
            }
            else {
                cpu->PC += 2;
                break;
            }
        }

        case JNC: {     // jump if carry flat is not set
            if (!is_flag_set(cpu->FLAGS, FLAG_CARRY)) {
                uint16_t addr = ram_read(ram, cpu->PC++) << 8;
                addr |= ram_read(ram, cpu->PC++);
                cpu->PC = addr;
                break;
            }
            else {
                cpu->PC += 2;
                break;
            }
        }

        case JE: {      // jump if equal (CMP)
            if (is_flag_set(cpu->FLAGS, FLAG_ZERO)) {
                uint16_t addr = ram_read(ram, cpu->PC++) << 8;
                addr |= ram_read(ram, cpu->PC++);
                cpu->PC = addr;
                break;
            }
            else {
                cpu->PC += 2;
                break;
            }
        }

        case JNE: {     // jump if not equal (CMP)
            if (!is_flag_set(cpu->FLAGS, FLAG_ZERO)) {
                uint16_t addr = ram_read(ram, cpu->PC++) << 8;
                addr |= ram_read(ram, cpu->PC++);
                cpu->PC = addr;
                break;
            }
            else {
                cpu->PC += 2;
                break;
            }
        }

        case JL: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);

            bool sf = is_flag_set(cpu->FLAGS, FLAG_SIGN);
            bool of = is_flag_set(cpu->FLAGS, FLAG_OVERFLOW);

            if (sf != of) {
                cpu->PC = addr;
            }
            break;
        }

        case JG: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);

            bool sf = is_flag_set(cpu->FLAGS, FLAG_SIGN);
            bool of = is_flag_set(cpu->FLAGS, FLAG_OVERFLOW);
            bool zf = is_flag_set(cpu->FLAGS, FLAG_ZERO);

            if (!zf && (sf == of)) {
                cpu->PC = addr;
            }
            break;
        }

        case JB: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);

            if (is_flag_set(cpu->FLAGS, FLAG_CARRY)) {
                cpu->PC = addr;
            }
            break;
        }

        case JA: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);

            bool cf = is_flag_set(cpu->FLAGS, FLAG_CARRY);
            bool zf = is_flag_set(cpu->FLAGS, FLAG_ZERO);

            if (!cf && !zf) {
                cpu->PC = addr;
            }
            break;
        }

        case AND: {
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint8_t result = a & b;

            set_flags_bitwise_ops(cpu, result);
            cpu->registers[reg_to] = result;
            break;
        }

        case OR: {
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint8_t result = a | b;

            set_flags_bitwise_ops(cpu, result);
            cpu->registers[reg_to] = result;
            break;
        }

        case XOR: {
            uint8_t reg_to = ram_read(ram, cpu->PC++);
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint8_t result = a ^ b;

            set_flags_bitwise_ops(cpu, result);
            cpu->registers[reg_to] = result;
            break;
        }

        case NOT: {
            uint8_t reg_not = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_not)) exit(1);

            uint8_t result = ~cpu->registers[reg_not];

            set_flags_bitwise_ops(cpu, result);
            cpu->registers[reg_not] = result;
            break;
        }

        case PUSH: {
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint16_t value = cpu->registers[reg_from];
            ram_write(ram, --cpu->SP, value);
            break;
        }

        case POP: {
            uint8_t reg_to = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);

            uint16_t value = ram_read(ram, cpu->SP++);
            cpu->registers[reg_to] = value;
            break;
        }

        case CALL: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);

            // save return address: push PC onto stack
            uint16_t value = cpu->PC;
            uint8_t valHI = (value >> 8) & 0xFF;
            uint8_t valLO = value & 0xFF;
            ram_write(ram, --cpu->SP, valLO);
            ram_write(ram, --cpu->SP, valHI);

            cpu->PC = addr;
            break;
        }

        case RET: {
            // 16 bit pop
            uint16_t PC_addr = ram_read(ram, cpu->SP++) << 8;
            PC_addr |= ram_read(ram, cpu->SP++);

            cpu->PC = PC_addr;
            break;
        }

        case HLT:       // end of program
            cpu->halted = true;
            break;

        default:
            cpu->halted = true;
            break;

    }
}