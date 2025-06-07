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

// REGISTER BOUNDS CHECK
bool register_out_of_bounds(CPU* cpu, uint8_t registers) {
    size_t array_elems = sizeof(cpu->registers) / sizeof(cpu->registers[0]);

    if (registers >= array_elems) {
        return true;
    }

    return false;
}


//CPU
void cpu_reset(CPU *cpu) {
    cpu->registers[A] = cpu->registers[B] = 0;
    cpu->PC = 0x0000;
    cpu->SP = 0x0000;
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
            cpu->registers[A] = result;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case DEC: {      // not updating the carry flag on purpose
            uint16_t result = cpu->registers[A] - 1;
            cpu->registers[A] = result;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case ADD: {
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint16_t result = cpu->registers[A] + cpu->registers[reg_from];

            if (result > 0xFF) set_flag(&cpu->FLAGS, FLAG_CARRY);
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            cpu->registers[A] = (uint8_t)result;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case SUB: {
            uint8_t reg_from = ram_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint16_t result = cpu->registers[A] - cpu->registers[reg_from];

            if (cpu->registers[0] < cpu->registers[reg_from]) { // borrow
                set_flag(&cpu->FLAGS, FLAG_CARRY);
            }
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            cpu->registers[A] = (uint8_t)result;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case MUL: {
            uint8_t reg_from = ram_read(ram, cpu->PC++);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint16_t result = cpu->registers[A] * cpu->registers[B];

            if ((result & 0xFF00) != 0) {
                set_flag(&cpu->FLAGS, FLAG_CARRY);  // overflow into high byte
            }
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            cpu->registers[A] = (uint8_t)(result & 0xFF);   // store low byte

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
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

        case CMP: {     // do not overwrite A
            uint16_t result = cpu->registers[A] - cpu->registers[B];

            if (result > 0xFF) set_flag(&cpu->FLAGS, FLAG_CARRY);
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            if (result == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
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

        case HLT:       // end of program
            cpu->halted = true;
            break;

        default:
            cpu->halted = true;
            break;

    }
}