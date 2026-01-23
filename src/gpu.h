#ifndef EMULATOR_GPU_H
#define EMULATOR_GPU_H

#include <stdbool.h>
#include "ram.h"

// if true, a gpu render is pending
extern bool gpu_dirty;

void gpu_render(RAM* ram);   // very abstract gpu render function

#endif //EMULATOR_GPU_H