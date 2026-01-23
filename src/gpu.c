#include <stdio.h>
#include <ctype.h>
#include "gpu.h"
#include "bus.h"

bool gpu_dirty = false;

// 32x8 Screen Renderer
void gpu_render(RAM* ram) {
    printf("\033[H");

    // border
    printf("┌────────────────────────────────┐\n");

    for (int y = 0; y < 8; y++) {
        printf("│");
        for (int x = 0; x < 32; x++) {
            uint16_t offset = (y * 32) + x;
            uint16_t address = MMIO_GPU_START + offset;

            uint8_t pixel = bus_read(ram, address);

            char c;
            if (isprint(pixel)) {
                c = (char)pixel;
            } else {
                c = '.';        // substitute
            }
            putchar(c);
        }
        printf("│\n");
    }

    printf("└────────────────────────────────┘\n");
    fflush(stdout);
}
