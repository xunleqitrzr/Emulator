#include <stdio.h>

#include "cpu.h"
#include "ram.h"
#include "rom.h"

int main(void) {
    CPU cpu;
    RAM ram;

    cpu_reset(&cpu);
    ram_init(&ram);

    uint8_t program[] = {
        LDI, 5,
        MOV, B, A,
        LDI, 0,
        INC,
        PUSH, A,
        CMP, B, A,
        JE, 0x00, 0x13,
        JMP, 0x00, 0x07,
        HLT
    };

    rom_load(&ram, program, sizeof(program));

    while (!cpu.halted) {
        cpu_step(&cpu, &ram);
    }

    print_state(&cpu);
    return 0;
}