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
        LDI, 2,
        HLT
    };

    rom_load(&ram, program, sizeof(program));

    while (!cpu.halted) {
        cpu_step(&cpu, &ram);
    }

    printf("10 divided by 2 is: %d\n", ram_read(&ram, 0x0008));

    return 0;
}