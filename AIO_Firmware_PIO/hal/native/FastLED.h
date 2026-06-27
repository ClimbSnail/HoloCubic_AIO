#ifndef FASTLED_STUB_H
#define FASTLED_STUB_H

// 阻止真实的 lib/FastLED 库头文件被包含
#define __INC_FASTSPI_LED2_H
#define __INC_PIXELS_H
#define __INC_CONTROLLER_H
#define __INC_COLORUTILS_H
#define __INC_LIB8TION_H
#define __INC_HSV2RGB_H
#define __INC_NOISE_H
#define __INC_POWER_MGT_H
#define __INC_DMX_H
#define __INC_COLORPALETTES_H
#define __INC_FASTPIN_H
#define __INC_FASTSPI_H
#define __INC_FASTSPI_TYPES_H
#define __INC_CHIPSETS_H
#define __INC_BITSWAP_H
#define __INC_CPP_COMPAT_H
#define __INC_PIXELSET_H
#define __INC_COLOR_H
#define __INC_LED_SYSDEFS_H
#define __INC_FASTLED_CONFIG_H
#define __INC_FASTLED_DELAY_H
#define __INC_FASTLED_PROGMEM_H

#include <stdint.h>

struct CRGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    CRGB() : r(0), g(0), b(0) {}
    CRGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    CRGB &setRGB(uint8_t nr, uint8_t ng, uint8_t nb) { r = nr; g = ng; b = nb; return *this; }
};

#endif /* FASTLED_STUB_H */