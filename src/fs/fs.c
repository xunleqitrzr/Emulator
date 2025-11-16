#include "fs.h"
#include <stdlib.h>
#include <stdio.h>

// load a binary file into RAM
uint16_t load_program_from_file(RAM* ram, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open program file %s\n", filename);
        exit(1);
    }

    // seek to the end of the file to find its size
    fseek(f, 0, SEEK_END);
    long fsize_long = ftell(f);
    if ((fsize_long < 0) || (fsize_long >= (long) UINT16_MAX)) exit(1);
    uint16_t fsize = (uint16_t) fsize_long;
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

    return fsize;
}
