#include "cpu.h"

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
    if (cpu->A == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
    else clear_flag(&cpu->FLAGS, FLAG_ZERO);
}

void cpu_reset(CPU *cpu) {
    cpu->A = cpu->B = 0;
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
            cpu->A = ram_read(ram, addr);
            break;
        }

        case LDB: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            cpu->B = ram_read(ram, addr);
            break;
        }

        case LDI: {     // same as LDA for now
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            cpu->A = ram_read(ram, addr);
            break;
        }

        case INC: {      // not updating the carry flag on purpose
            uint16_t result = cpu->A + 1;
            cpu->A = result;

            if (cpu->A == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case DEC: {      // not updating the carry flag on purpose
            uint16_t result = cpu->A - 1;
            cpu->A = result;

            if (cpu->A == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case ADD: {
            uint16_t result = cpu->A + cpu->B;

            if (result > 0xFF) set_flag(&cpu->FLAGS, FLAG_CARRY);
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            cpu->A = (uint8_t)result;

            if (cpu->A == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case SUB: {
            uint16_t result = cpu->A - cpu->B;

            if (result > 0xFF) set_flag(&cpu->FLAGS, FLAG_CARRY);
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            cpu->A = (uint8_t)result;

            if (cpu->A == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case MUL: {
            uint16_t result = cpu->A * cpu->B;

            if (result > 0xFF) set_flag(&cpu->FLAGS, FLAG_CARRY);
            else clear_flag(&cpu->FLAGS, FLAG_CARRY);

            cpu->A = (uint8_t)result;

            if (cpu->A == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case STA: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            ram_write(ram, addr, cpu->A);
            break;
        }

        case STB: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            ram_write(ram, addr, cpu->B);
            break;
        }

        case MOV:       // move B register into A: MOV A, B
            cpu->A = cpu->B;
            break;

        case CMP: {     // do not overwrite A
            uint16_t result = cpu->A - cpu->B;

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