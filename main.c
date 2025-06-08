#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "ram.h"
#include "rom.h"

int main(void) {
    CPU cpu;
    RAM ram;

    cpu_reset(&cpu);
    ram_init(&ram);

    uint8_t program[] = {
        LDI, 1,
        PUSH, A,
        CALL, 0x00, 0x08,
        HLT,
        NOP, NOP,
        RET,
        HLT
    };

    rom_load(&ram, program, sizeof(program));

    while (!cpu.halted) {
        cpu_step(&cpu, &ram);
    }

    return 0;
}