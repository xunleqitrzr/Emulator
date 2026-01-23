#include "cpu.h"
#include "bus.h"
#include <stdio.h>
#include <stdlib.h>

#include "security/security.h"

// FLAG LOGIC
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
void set_flags_add(CPU* cpu, uint8_t reg1, uint8_t reg2, uint16_t result) {
    SET_FLAG_IF(cpu, (uint8_t)result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, result > 0xFF, FLAG_CARRY);
    SET_FLAG_IF(cpu, (result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(cpu, ((reg1 ^ result) & (reg2 ^ result)) & 0x80, FLAG_OVERFLOW);
}

void set_flags_sub(CPU* cpu, uint8_t reg1, uint8_t reg2, uint16_t result) {
    SET_FLAG_IF(cpu, (uint8_t)result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, reg1 < reg2, FLAG_CARRY);
    SET_FLAG_IF(cpu, (result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(cpu, ((reg1 ^ reg2) & (reg1 ^ result)) & 0x80, FLAG_OVERFLOW);
}

void set_flags_inc(CPU* cpu, uint8_t original, uint16_t result) {
    SET_FLAG_IF(cpu, (uint8_t)result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, (result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(cpu, original == 0x7F, FLAG_OVERFLOW);
    // CF unaffected
}

void set_flags_dec(CPU* cpu, uint8_t original, uint16_t result) {
    SET_FLAG_IF(cpu, (uint8_t)result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, (result & 0x80), FLAG_SIGN);
    SET_FLAG_IF(cpu, original == 0x80, FLAG_OVERFLOW);
    // CF unaffected
}

void set_flags_bitwise_ops(CPU* cpu, uint8_t result) {
    SET_FLAG_IF(cpu, result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, (result & 0x80), FLAG_SIGN);
    // CF and OF cleared on these instructions on many CPUs
    clear_flag(&cpu->FLAGS, FLAG_CARRY);
    clear_flag(&cpu->FLAGS, FLAG_OVERFLOW);
}

void set_flags_mul(CPU* cpu, uint16_t result) {
    uint8_t low  = result & 0xFF;
    uint8_t high = (result >> 8) & 0xFF;

    SET_FLAG_IF(cpu, low == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, low & 0x80, FLAG_SIGN);

    bool overflow = (high != 0);
    SET_FLAG_IF(cpu, overflow, FLAG_CARRY);
    SET_FLAG_IF(cpu, overflow, FLAG_OVERFLOW);
}

void set_flags_shift_left(CPU* cpu, uint8_t original, uint8_t result) {
    SET_FLAG_IF(cpu, original & 0x80, FLAG_CARRY);
    SET_FLAG_IF(cpu, result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, result & 0x80, FLAG_SIGN);
}
void set_flags_shift_right(CPU* cpu, uint8_t original, uint8_t result) {
    SET_FLAG_IF(cpu, original & 0x01, FLAG_CARRY);
    SET_FLAG_IF(cpu, result == 0, FLAG_ZERO);
    SET_FLAG_IF(cpu, result & 0x80, FLAG_SIGN);
    // no overflow, SHR is logical
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

uint16_t get_stack_pointer(CPU* cpu) {
    return cpu->SP;
}

// DEBUG
void print_state(CPU* cpu) {
    // registers
    printf("A:%d B:%d C:%d D:%d\n",
        cpu->registers[0],
        cpu->registers[1],
        cpu->registers[2],
        cpu->registers[3]);

    // program counter
    printf("PC: d:%d h:0x%08x\n",
        cpu->PC, cpu->PC);

    // stack pointer
    printf("SP: d:%d h:0x%08x\n",
        cpu->SP, cpu->SP);

    print_flags(cpu->FLAGS);
}

void print_flags(uint8_t flags) {
    printf("Z:%d S:%d C:%d O:%d\n",
        !!(flags & FLAG_ZERO),
        !!(flags & FLAG_SIGN),
        !!(flags & FLAG_CARRY),
        !!(flags & FLAG_OVERFLOW));
}

//CPU
void cpu_reset(CPU *cpu) {
    for (size_t i = 0; i < get_number_of_registers(cpu); i++) {
        cpu->registers[i] = 0;
    }
    cpu->PC = 0x0000;
    cpu->SP = RAM_SIZE - 1;     // 0xFFFF
    cpu->FLAGS = 0;
    cpu->halted = false;
}

void cpu_step(CPU* cpu, RAM* ram) {
    if (cpu->halted) return;

    uint8_t opcode = bus_read(ram, cpu->PC++);

    switch (opcode) {
        case NOP:
            break;

        case LDA: {
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;
            addr |= bus_read(ram, cpu->PC++);
            cpu->registers[A] = bus_read(ram, addr);

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case LDB: {
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;
            addr |= bus_read(ram, cpu->PC++);
            cpu->registers[B] = bus_read(ram, addr);
            break;
        }

        case LDI: {
            uint8_t immediate_value = bus_read(ram, cpu->PC++);
            cpu->registers[A] = immediate_value;

            if (cpu->registers[A] == 0) set_flag(&cpu->FLAGS, FLAG_ZERO);
            else clear_flag(&cpu->FLAGS, FLAG_ZERO);
            break;
        }

        case INC: {      // not updating the carry flag on purpose
            uint8_t reg_inc = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_inc)) exit(1);

            uint16_t original = cpu->registers[reg_inc];
            uint16_t result = original + 1;

            set_flags_inc(cpu, original, result);
            cpu->registers[reg_inc] = (uint8_t) result;
            break;
        }

        case DEC: {      // not updating the carry flag on purpose
            uint8_t reg_dec = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_dec)) exit(1);

            uint16_t original = cpu->registers[reg_dec];
            uint16_t result = original - 1;

            set_flags_dec(cpu, original, result);
            cpu->registers[reg_dec] = (uint8_t) result;
            break;
        }

        case ADD: {     // ADD C, B
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint16_t result = (uint16_t)a + (uint16_t)b;

            set_flags_add(cpu, a, b, result);
            cpu->registers[reg_to] = (uint8_t)result;
            break;
        }

        case SUB: {     // SUB C, B
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint16_t result = (uint16_t)a - (uint16_t)b;

            set_flags_sub(cpu, a, b, result);
            cpu->registers[reg_to] = (uint8_t)result;
            break;
        }

        case MUL: {     // MUL D, B
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint16_t a = (uint16_t)cpu->registers[reg_to];
            uint16_t b = (uint16_t)cpu->registers[reg_from];
            uint16_t result = a * b;

            set_flags_mul(cpu, result);
            cpu->registers[reg_to] = (uint8_t) (result & 0xFF); // store low
            break;
        }

        case STA: {
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;
            addr |= bus_read(ram, cpu->PC++);
            bus_write(ram, addr, cpu->registers[A]);
            break;
        }

        case STB: {
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;
            addr |= bus_read(ram, cpu->PC++);
            bus_write(ram, addr, cpu->registers[B]);
            break;
        }

        case MOV: {     // move B register into A: MOV A, B
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t value = cpu->registers[reg_from];
            cpu->registers[reg_to] = value;
            set_flags_bitwise_ops(cpu, value);
            break;
        }

        case CMP: {     // CMP B, D
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);
            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            uint8_t a = cpu->registers[reg_to];
            uint8_t b = cpu->registers[reg_from];
            uint16_t result = (uint16_t)a - (uint16_t)b;

            set_flags_sub(cpu, a, b, result);
            break;
        }

        #define JUMP_IF(condition) \
            uint16_t addr = bus_read(ram, cpu->PC++) << 8; \
            addr |= bus_read(ram, cpu->PC++); \
            if (condition) cpu->PC = addr

        case JMP: {     // unconditional jump
            JUMP_IF(true);
            break;
        }

        case JZ: {      // jump if zero
            JUMP_IF(is_flag_set(cpu->FLAGS, FLAG_ZERO));
            break;
        }

        case JNZ: {     // jump if not zero
            JUMP_IF(!is_flag_set(cpu->FLAGS, FLAG_ZERO));
            break;
        }

        case JC: {      // jump if carry flag is set
            JUMP_IF(is_flag_set(cpu->FLAGS, FLAG_CARRY));
            break;
        }

        case JNC: {     // jump if carry flat is not set
            JUMP_IF(!is_flag_set(cpu->FLAGS, FLAG_CARRY));
            break;
        }

        case JE: {      // jump if equal (CMP)
            JUMP_IF(is_flag_set(cpu->FLAGS, FLAG_ZERO));
            break;
        }

        case JNE: {     // jump if not equal (CMP)
            JUMP_IF(!is_flag_set(cpu->FLAGS, FLAG_ZERO));
            break;
        }

        case JL: {
            // for readability
            bool sf = is_flag_set(cpu->FLAGS, FLAG_SIGN);
            bool of = is_flag_set(cpu->FLAGS, FLAG_OVERFLOW);

            JUMP_IF(sf != of);
            break;
        }

        case JLE: {
            bool sf = is_flag_set(cpu->FLAGS, FLAG_SIGN);
            bool of = is_flag_set(cpu->FLAGS, FLAG_OVERFLOW);
            bool zf = is_flag_set(cpu->FLAGS, FLAG_ZERO);

            JUMP_IF(zf || (sf != of));
            break;
        }

        case JG: {
            bool sf = is_flag_set(cpu->FLAGS, FLAG_SIGN);
            bool of = is_flag_set(cpu->FLAGS, FLAG_OVERFLOW);
            bool zf = is_flag_set(cpu->FLAGS, FLAG_ZERO);

            JUMP_IF(!zf && (sf == of));
            break;
        }

        case JGE: {
            bool sf = is_flag_set(cpu->FLAGS, FLAG_SIGN);
            bool of = is_flag_set(cpu->FLAGS, FLAG_OVERFLOW);

            JUMP_IF(sf == of);
            break;
        }

        case JB: {
            JUMP_IF(is_flag_set(cpu->FLAGS, FLAG_CARRY));
            break;
        }

        case JA: {
            bool cf = is_flag_set(cpu->FLAGS, FLAG_CARRY);
            bool zf = is_flag_set(cpu->FLAGS, FLAG_ZERO);

            JUMP_IF(!cf && !zf);
            break;
        }

        case AND: {
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

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
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

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
            uint8_t reg_to = bus_read(ram, cpu->PC++);
            uint8_t reg_from = bus_read(ram, cpu->PC++);

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
            uint8_t reg_not = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_not)) exit(1);

            uint8_t result = ~cpu->registers[reg_not];

            set_flags_bitwise_ops(cpu, result);
            cpu->registers[reg_not] = result;
            break;
        }

        case PUSH: {
            uint8_t reg_from = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_from)) exit(1);

            // safety check: stack overflow
            // ensure stack does not overwrite program code
            if ((cpu->SP - 1) <= get_usable_offset()) {
                fprintf(stderr, "Stack Overflow! SP collided with program at 0x%04X\n", cpu->SP);
                exit(1);
            }

            uint8_t value = cpu->registers[reg_from];
            bus_write_system(ram, --cpu->SP, value);
            break;
        }

        case POP: {
            uint8_t reg_to = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_to)) exit(1);

            uint8_t value = bus_read(ram, cpu->SP++);
            cpu->registers[reg_to] = value;
            break;
        }

        case CALL: {
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;
            addr |= bus_read(ram, cpu->PC++);

            // safety check: stack overflow
            // ensure stack does not overwrite program code
            if ((cpu->SP - 2) <= get_usable_offset()) {
                fprintf(stderr, "Stack Overflow during CALL at 0x%04X\n", cpu->SP);
                exit(1);
            }

            // save return address: push PC onto stack
            uint16_t value = cpu->PC;
            uint8_t valHI = (value >> 8) & 0xFF;
            uint8_t valLO = value & 0xFF;
            bus_write_system(ram, --cpu->SP, valLO);
            bus_write_system(ram, --cpu->SP, valHI);

            cpu->PC = addr;
            break;
        }

        case RET: {
            // 16 bit pop
            uint16_t PC_addr = bus_read(ram, cpu->SP++) << 8;
            PC_addr |= bus_read(ram, cpu->SP++);

            cpu->PC = PC_addr;
            break;
        }

        case SHL: {     // SHL <register>
            uint8_t reg_shl = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_shl)) exit(1);

            uint8_t original = cpu->registers[reg_shl];
            uint8_t result = original << 1;

            // flag logic
            set_flags_shift_left(cpu, original, result);
            SET_FLAG_IF(cpu, result & 0x80, FLAG_OVERFLOW);
            cpu->registers[reg_shl] = result;
            break;
        }

        case SHR: {     // SHR <register>
            uint8_t reg_shr = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_shr)) exit(1);

            uint8_t original = cpu->registers[reg_shr];
            uint8_t result = original >> 1;

            // flag logic
            set_flags_shift_right(cpu, original, result);
            cpu->registers[reg_shr] = result;
            break;
        }

        case ROL: {     // ROL <register>
            uint8_t reg_rol = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_rol)) exit(1);

            uint8_t original = cpu->registers[reg_rol];
            bool old_carry = is_flag_set(cpu->FLAGS, FLAG_CARRY);

            uint8_t result = (original << 1) | (old_carry ? 0x01 : 0x00);

            set_flags_shift_left(cpu, original, result);
            cpu->registers[reg_rol] = result;
            break;
        }

        case ROR: {     // ROR <register>
            uint8_t reg_ror = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_ror)) exit(1);

            uint8_t original = cpu->registers[reg_ror];
            bool old_carry = is_flag_set(cpu->FLAGS, FLAG_CARRY);

            uint8_t result = (original >> 1) | (old_carry ? 0x80 : 0x00);

            // flag logic
            set_flags_shift_right(cpu, original, result);
            cpu->registers[reg_ror] = result;
            break;
        }

        case LDA_IDX: {     // 4 byte: [OPCODE] [ADDR_HI] [ADD_LO] [REG]
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;          // read three bytes
            addr |= bus_read(ram, cpu->PC++);
            uint8_t reg_idx = bus_read(ram, cpu->PC++);             // this one is the index register

            if (register_out_of_bounds(cpu, reg_idx)) exit(1);

            uint16_t eff_addr = addr + cpu->registers[reg_idx];     // calculate effective address in memory
            cpu->registers[A] = bus_read(ram, eff_addr);            // read from that memory
            break;
        }

        case STA_IDX: {     // 4 byte: [OPCODE] [ADDR_HI] [ADD_LO] [REG]
            uint16_t addr = bus_read(ram, cpu->PC++) << 8;
            addr |= bus_read(ram, cpu->PC++);
            uint8_t reg_idx = bus_read(ram, cpu->PC++);

            if (register_out_of_bounds(cpu, reg_idx)) exit(1);

            uint16_t eff_addr = addr + cpu->registers[reg_idx];
            bus_write(ram, eff_addr, cpu->registers[A]);
            break;
        }

        case HLT:       // end of program
            cpu->halted = true;
            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%02X\n", opcode);
            cpu->halted = true;
            break;

    }
}