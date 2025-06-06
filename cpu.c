#include "cpu.h"

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

        case ADD:
            cpu->A += cpu->B;
            break;

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

        case JMP: {
            uint16_t addr = ram_read(ram, cpu->PC++) << 8;
            addr |= ram_read(ram, cpu->PC++);
            cpu->PC = addr;
            break;
        }

        case HLT:
            cpu->halted = true;
            break;

        default:
            cpu->halted = true;
            break;

    }
}