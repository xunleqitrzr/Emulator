#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "gpu.h"
#include "fs/fs.h"
#include "security/security.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        exit(1);
    }

    const char* file_name = argv[1];

    CPU cpu;
    RAM ram;

    cpu_reset(&cpu);
    ram_init(&ram);
    security_init(&cpu);

    printf("Loading \"%s\" into memory...\n", file_name);
    const uint16_t size = load_program_from_file(&ram, file_name);
    set_usable_offset(size);
    printf("Load complete. Starting CPU...\n");

    printf("\033[2J");

    while (!cpu.halted) {
        cpu_step(&cpu, &ram);
        if (gpu_dirty) { gpu_render(&ram); gpu_dirty = false; }
    }

    print_state(&cpu);
    return 0;
}