#include "../Arduino_DataBus.h"
#if !defined(LITTLE_FOOT_PRINT)

#include "../Arduino_GFX.h"
#include "Arduino_Canvas_Mono.h"

Arduino_Canvas_Mono::Arduino_Canvas_Mono(int16_t w, int16_t h, Arduino_G *output, int16_t output_x, int16_t output_y)
    : Arduino_GFX(w, h), _output(output), _output_x(output_x), _output_y(output_y)
{
}

void Arduino_Canvas_Mono::begin(int32_t speed)
{
    size_t s = (_width * _height +  7) / 8;
#if defined(ESP32)
    if (psramFound())
    {
        _framebuffer = (uint8_t *)ps_malloc(s);
    }
    else
    {
        _framebuffer = (uint8_t *)malloc(s);
    }
#else
    _framebuffer = (uint8_t *)malloc(s);
#endif
    if (!_framebuffer)
    {
        Serial.println(F("_framebuffer allocation failed."));
    }

    _output->begin(speed);
}

void Arduino_Canvas_Mono::writePixelPreclipped(int16_t x, int16_t y, uint16_t color)
{
    int32_t pos = x + (y * _width);
    if (color & 0b1000010000010000)
    {
        _framebuffer[pos >> 3] |= 0x80 >> (pos & 7);
    }
    else
    {
        _framebuffer[pos >> 3] &= ~(0x80 >> (pos & 7));
    }
}

void Arduino_Canvas_Mono::flush()
{
    _output->drawBitmap(_output_x, _output_y, _framebuffer, _width, _height, WHITE, BLACK);
}

#endif // !defined(LITTLE_FOOT_PRINT)
