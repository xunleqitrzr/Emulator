#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "ram.h"
#include "rom.h"

// load a binary file into RAM
void load_program_from_file(RAM* ram, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open program file %s\n", filename);
        exit(1);
    }

    // seek to the end of the file to find its size
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  // back to beginning

    // read the entire file into a temporary buffer
    unsigned char* buffer = (unsigned char*)malloc(fsize);
    if (!buffer) {
        fprintf(stderr, "Error: Could not allocate program memory\n");
        fclose(f);
        exit(1);
    }

    fread(buffer, fsize, 1, f);
    fclose(f);

    // copy program from buffer to emulator ROM
    rom_load(ram, buffer, (uint16_t) fsize);

    // clean up
    free(buffer);
}

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