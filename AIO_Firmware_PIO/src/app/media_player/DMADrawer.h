#ifndef _DMADRAWER_H_
#define _DMADRAWER_H_

#include <stdint.h>

struct DMADrawer
{
    static bool setup(uint32_t bufsize, int spi_freq, int tft_mosi, int tft_miso, int tft_sclk, int tft_cs, int tft_dc);
    static void close();
    static uint16_t *getNextBuffer();
    static void draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    static void waitDraw();
};

#endif
