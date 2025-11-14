#include "cpu.h"
#include "fs/fs.h"


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

    printf("Loading \"%s\" into memory...\n", file_name);
    load_program_from_file(&ram, file_name);
    printf("Load complete. Starting CPU...\n");

    while (!cpu.halted) {
        cpu_step(&cpu, &ram);
    }

    print_state(&cpu);
    return 0;
}