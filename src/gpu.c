#include <stdio.h>
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
            uint16_t address = MMIO_VISUAL_BEGIN + offset;

            uint8_t pixel = bus_read(ram, address);

            char c = (pixel == 0) ? ' ' : (char)pixel;
            putchar(c);
        }
        printf("│\n");
    }

    printf("└────────────────────────────────┘\n");
    fflush(stdout);
}
