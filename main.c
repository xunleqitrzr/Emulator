#include <stdio.h>
#include "cpu.h"
#include "ram.h"
#include "rom.h"

int main(void) {
    CPU cpu;
    RAM ram;

    cpu_reset(&cpu);
    ram_init(&ram);

    // sample program:
    // LDA 0x0010       ; load A from address 0x0010 (value = 42)
    // ADD              ; A = A + B (B = 10), because there is only one other register than A, ADD is simply enough to add B to A
    // STA 0x0020       ; store A to 0x0020
    // HLT              ; end the program


    /*
    uint8_t program[] = {
        LDA, 0x00, 0x10,
        ADD,
        STA, 0x00, 0x20,
        HLT
    };

    rom_load(&ram, program, sizeof(program));
    ram_write(&ram, 0x0010, 42);
    cpu.B = 10;
    */

    // uint8_t program[] = {
    //     LDA, 0x00, 0x20,
    //     LDB, 0x00, 0x21,
    //     ADD,
    //     STA, 0x00, 0x22,
    //     LDB, 0x00, 0x22,
    //     ADD,
    //     STA, 0x00, 0x23,
    //     JMP, 0x00, 0x00,
    //     HLT
    // };

    uint8_t program[] = {
        LDA, 0x00, 0x20,
        LDB, 0x00, 0x21,
        ADD,
        STA, 0x00, 0x22,
        HLT
    };

    rom_load(&ram, program, sizeof(program));
    ram_write(&ram, 0x0020, 255);
    ram_write(&ram, 0x0021, 1);

    while (!cpu.halted) {
        cpu_step(&cpu, &ram);
    }

    printf("Result at 0x0022: %d\n", ram_read(&ram, 0x0022));
    return 0;
}