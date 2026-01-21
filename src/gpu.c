#include <stdio.h>
#include "gpu.h"
#include "ram.h"

void gpu_render(RAM* ram) {
    printf("\033[H");           // move cursor to top left

    for (uint16_t i = MMIO_VISUAL_BEGIN; i <= MMIO_VISUAL_END; i++) {
        char c = ram_read(ram, i);
        putchar(c ? c : ' ');

        // newline every 16 chars
        if ((i - MMIO_VISUAL_BEGIN + 1) % 16 == 0) {
            putchar('\n');
        }
    }

    fflush(stdout);
}
