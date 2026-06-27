#include "hal_native.h"
#include <stdio.h>

void hal_rgb_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    printf("[RGB] LED set to R=%d G=%d B=%d\n", r, g, b);
}