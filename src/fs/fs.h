#ifndef FS_H
#define FS_H

#include "../ram.h"
#include "../rom.h"

uint16_t load_program_from_file(RAM* ram, const char* filename);

#endif