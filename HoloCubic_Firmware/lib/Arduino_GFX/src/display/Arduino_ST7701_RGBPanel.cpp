#include "../Arduino_DataBus.h"

#if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

#include "../Arduino_GFX.h"
#include "Arduino_ST7701_RGBPanel.h"

Arduino_ST7701_RGBPanel::Arduino_ST7701_RGBPanel(
    Arduino_ESP32RGBPanel *bus, int8_t rst, int16_t w, int16_t h)
    : Arduino_GFX(w, h), _bus(bus), _rst(rst)
{
}

void Arduino_ST7701_RGBPanel::begin(int32_t speed)
{
    _bus->begin(speed);

    _bus->sendCommand(0xFF);
    _bus->sendData(0x77);
    _bus->sendData(0x01);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x10);

    _bus->sendCommand(0xC0);
    _bus->sendData(0x3B);
    _bus->sendData(0x00);

    _bus->sendCommand(0xC1);
    _bus->sendData(0x0D);
    _bus->sendData(0x02);

    _bus->sendCommand(0xC2);
    _bus->sendData(0x31);
    _bus->sendData(0x05);

    _bus->sendCommand(0xCd);
    _bus->sendData(0x08);

    _bus->sendCommand(0xB0);
    _bus->sendData(0x00); // Positive Voltage Gamma Control
    _bus->sendData(0x11);
    _bus->sendData(0x18);
    _bus->sendData(0x0E);
    _bus->sendData(0x11);
    _bus->sendData(0x06);
    _bus->sendData(0x07);
    _bus->sendData(0x08);
    _bus->sendData(0x07);
    _bus->sendData(0x22);
    _bus->sendData(0x04);
    _bus->sendData(0x12);
    _bus->sendData(0x0F);
    _bus->sendData(0xAA);
    _bus->sendData(0x31);
    _bus->sendData(0x18);

    _bus->sendCommand(0xB1);
    _bus->sendData(0x00); // Negative Voltage Gamma Control
    _bus->sendData(0x11);
    _bus->sendData(0x19);
    _bus->sendData(0x0E);
    _bus->sendData(0x12);
    _bus->sendData(0x07);
    _bus->sendData(0x08);
    _bus->sendData(0x08);
    _bus->sendData(0x08);
    _bus->sendData(0x22);
    _bus->sendData(0x04);
    _bus->sendData(0x11);
    _bus->sendData(0x11);
    _bus->sendData(0xA9);
    _bus->sendData(0x32);
    _bus->sendData(0x18);

    // PAGE1
    _bus->sendCommand(0xFF);
    _bus->sendData(0x77);
    _bus->sendData(0x01);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x11);

    _bus->sendCommand(0xB0);
    _bus->sendData(0x60); // Vop=4.7375v
    _bus->sendCommand(0xB1);
    _bus->sendData(0x32); // VCOM=32
    _bus->sendCommand(0xB2);
    _bus->sendData(0x07); // VGH=15v
    _bus->sendCommand(0xB3);
    _bus->sendData(0x80);
    _bus->sendCommand(0xB5);
    _bus->sendData(0x49); // VGL=-10.17v
    _bus->sendCommand(0xB7);
    _bus->sendData(0x85);
    _bus->sendCommand(0xB8);
    _bus->sendData(0x21); // AVDD=6.6 & AVCL=-4.6
    _bus->sendCommand(0xC1);
    _bus->sendData(0x78);
    _bus->sendCommand(0xC2);
    _bus->sendData(0x78);

    _bus->sendCommand(0xE0);
    _bus->sendData(0x00);
    _bus->sendData(0x1B);
    _bus->sendData(0x02);

    _bus->sendCommand(0xE1);
    _bus->sendData(0x08);
    _bus->sendData(0xA0);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x07);
    _bus->sendData(0xA0);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x44);
    _bus->sendData(0x44);

    _bus->sendCommand(0xE2);
    _bus->sendData(0x11);
    _bus->sendData(0x11);
    _bus->sendData(0x44);
    _bus->sendData(0x44);
    _bus->sendData(0xED);
    _bus->sendData(0xA0);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0xEC);
    _bus->sendData(0xA0);
    _bus->sendData(0x00);
    _bus->sendData(0x00);

    _bus->sendCommand(0xE3);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x11);
    _bus->sendData(0x11);

    _bus->sendCommand(0xE4);
    _bus->sendData(0x44);
    _bus->sendData(0x44);

    _bus->sendCommand(0xE5);
    _bus->sendData(0x0A);
    _bus->sendData(0xE9);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);
    _bus->sendData(0x0C);
    _bus->sendData(0xEB);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);
    _bus->sendData(0x0E);
    _bus->sendData(0xED);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);
    _bus->sendData(0x10);
    _bus->sendData(0xEF);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);

    _bus->sendCommand(0xE6);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x11);
    _bus->sendData(0x11);

    _bus->sendCommand(0xE7);
    _bus->sendData(0x44);
    _bus->sendData(0x44);

    _bus->sendCommand(0xE8);
    _bus->sendData(0x09);
    _bus->sendData(0xE8);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);
    _bus->sendData(0x0B);
    _bus->sendData(0xEA);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);
    _bus->sendData(0x0D);
    _bus->sendData(0xEC);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);
    _bus->sendData(0x0F);
    _bus->sendData(0xEE);
    _bus->sendData(0xD8);
    _bus->sendData(0xA0);

    _bus->sendCommand(0xEB);
    _bus->sendData(0x02);
    _bus->sendData(0x00);
    _bus->sendData(0xE4);
    _bus->sendData(0xE4);
    _bus->sendData(0x88);
    _bus->sendData(0x00);
    _bus->sendData(0x40);

    _bus->sendCommand(0xEC);
    _bus->sendData(0x3C);
    _bus->sendData(0x00);

    _bus->sendCommand(0xED);
    _bus->sendData(0xAB);
    _bus->sendData(0x89);
    _bus->sendData(0x76);
    _bus->sendData(0x54);
    _bus->sendData(0x02);
    _bus->sendData(0xFF);
    _bus->sendData(0xFF);
    _bus->sendData(0xFF);
    _bus->sendData(0xFF);
    _bus->sendData(0xFF);
    _bus->sendData(0xFF);
    _bus->sendData(0x20);
    _bus->sendData(0x45);
    _bus->sendData(0x67);
    _bus->sendData(0x98);
    _bus->sendData(0xBA);

    _bus->sendCommand(0x36);
    _bus->sendData(0x00);

    //-----------VAP & VAN---------------
    _bus->sendCommand(0xFF);
    _bus->sendData(0x77);
    _bus->sendData(0x01);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x13);

    _bus->sendCommand(0xE5);
    _bus->sendData(0xE4);

    _bus->sendCommand(0xFF);
    _bus->sendData(0x77);
    _bus->sendData(0x01);
    _bus->sendData(0x00);
    _bus->sendData(0x00);
    _bus->sendData(0x00);

    _bus->sendCommand(0x3A); // 0x70 RGB888, 0x60 RGB666, 0x50 RGB565
    _bus->sendData(0x60);

    _bus->sendCommand(0x21); // Display Inversion On

    _bus->sendCommand(0x11); // Sleep Out
    delay(100);

    _bus->sendCommand(0x29); // Display On
    delay(50);

    _framebuffer = _bus->getFrameBuffer(_width, _height);
}

void Arduino_ST7701_RGBPanel::writePixelPreclipped(int16_t x, int16_t y, uint16_t color)
{
    uint16_t *fb = _framebuffer;
    fb += (int32_t)y * _width;
    fb += x;
    MSB_16_SET(*fb, color);
    Cache_WriteBack_Addr((uint32_t)fb, 2);
}

void Arduino_ST7701_RGBPanel::writeFastVLine(int16_t x, int16_t y,
                                             int16_t h, uint16_t color)
{
    if (_ordered_in_range(x, 0, _max_x) && h)
    { // X on screen, nonzero height
        if (h < 0)
        {               // If negative height...
            y += h + 1; //   Move Y to top edge
            h = -h;     //   Use positive height
        }
        if (y <= _max_y)
        { // Not off bottom
            int16_t y2 = y + h - 1;
            if (y2 >= 0)
            { // Not off top
                // Line partly or fully overlaps screen
                if (y < 0)
                {
                    y = 0;
                    h = y2 + 1;
                } // Clip top
                if (y2 > _max_y)
                {
                    h = _max_y - y + 1;
                } // Clip bottom

                uint16_t *fb = _framebuffer + ((int32_t)y * _width) + x;
                while (h--)
                {
                    MSB_16_SET(*fb, color);
                    Cache_WriteBack_Addr((uint32_t)fb, 2);
                    fb += _width;
                }
            }
        }
    }
}

void Arduino_ST7701_RGBPanel::writeFastHLine(int16_t x, int16_t y,
                                             int16_t w, uint16_t color)
{
    if (_ordered_in_range(y, 0, _max_y) && w)
    { // Y on screen, nonzero width
        if (w < 0)
        {               // If negative width...
            x += w + 1; //   Move X to left edge
            w = -w;     //   Use positive width
        }
        if (x <= _max_x)
        { // Not off right
            int16_t x2 = x + w - 1;
            if (x2 >= 0)
            { // Not off left
                // Line partly or fully overlaps screen
                if (x < 0)
                {
                    x = 0;
                    w = x2 + 1;
                } // Clip left
                if (x2 > _max_x)
                {
                    w = _max_x - x + 1;
                } // Clip right

                uint16_t *fb = _framebuffer + ((int32_t)y * _width) + x;
                uint32_t cachePos = (uint32_t)fb;
                int16_t writeSize = w * 2;
                while (w--)
                {
                    MSB_16_SET(*(fb++), color);
                }
                Cache_WriteBack_Addr(cachePos, writeSize);
            }
        }
    }
}

void Arduino_ST7701_RGBPanel::writeFillRectPreclipped(int16_t x, int16_t y,
                                                      int16_t w, int16_t h, uint16_t color)
{
    uint16_t *row = _framebuffer;
    row += y * _width;
    uint32_t cachePos = (uint32_t)row;
    row += x;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            MSB_16_SET(row[i], color);
        }
        row += _width;
    }
    Cache_WriteBack_Addr(cachePos, _width * h * 2);
}

void Arduino_ST7701_RGBPanel::draw16bitRGBBitmap(int16_t x, int16_t y,
                                                 uint16_t *bitmap, int16_t w, int16_t h)
{
    if (
        ((x + w - 1) < 0) || // Outside left
        ((y + h - 1) < 0) || // Outside top
        (x > _max_x) ||      // Outside right
        (y > _max_y)         // Outside bottom
    )
    {
        return;
    }
    else
    {
        int16_t xskip = 0;
        if ((y + h - 1) > _max_y)
        {
            h -= (y + h - 1) - _max_y;
        }
        if (y < 0)
        {
            bitmap -= y * w;
            h += y;
            y = 0;
        }
        if ((x + w - 1) > _max_x)
        {
            xskip = (x + w - 1) - _max_x;
            w -= xskip;
        }
        if (x < 0)
        {
            bitmap -= x;
            xskip -= x;
            w += x;
            x = 0;
        }
        uint16_t *row = _framebuffer;
        row += y * _width;
        uint32_t cachePos = (uint32_t)row;
        row += x;
        uint16_t color;
        for (int j = 0; j < h; j++)
        {
            for (int i = 0; i < w; i++)
            {
                color = *bitmap++;
                MSB_16_SET(row[i], color);
            }
            bitmap += xskip;
            row += _width;
        }
        Cache_WriteBack_Addr(cachePos, _width * h * 2);
    }
}

void Arduino_ST7701_RGBPanel::draw16bitBeRGBBitmap(int16_t x, int16_t y,
                                                   uint16_t *bitmap, int16_t w, int16_t h)
{
    if (
        ((x + w - 1) < 0) || // Outside left
        ((y + h - 1) < 0) || // Outside top
        (x > _max_x) ||      // Outside right
        (y > _max_y)         // Outside bottom
    )
    {
        return;
    }
    else
    {
        int16_t xskip = 0;
        if ((y + h - 1) > _max_y)
        {
            h -= (y + h - 1) - _max_y;
        }
        if (y < 0)
        {
            bitmap -= y * w;
            h += y;
            y = 0;
        }
        if ((x + w - 1) > _max_x)
        {
            xskip = (x + w - 1) - _max_x;
            w -= xskip;
        }
        if (x < 0)
        {
            bitmap -= x;
            xskip -= x;
            w += x;
            x = 0;
        }
        uint16_t *row = _framebuffer;
        row += y * _width;
        uint32_t cachePos = (uint32_t)row;
        row += x;
        for (int j = 0; j < h; j++)
        {
            for (int i = 0; i < w; i++)
            {
                row[i] = *bitmap++;
            }
            bitmap += xskip;
            row += _width;
        }
        Cache_WriteBack_Addr(cachePos, _width * h * 2);
    }
}

#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)
