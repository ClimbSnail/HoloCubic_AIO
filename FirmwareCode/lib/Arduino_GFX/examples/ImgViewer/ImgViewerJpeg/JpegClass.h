/*******************************************************************************
 * JPEGDEC Wrapper Class
 * 
 * Dependent libraries:
 * JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 ******************************************************************************/
#ifndef _JPEGCLASS_H_
#define _JPEGCLASS_H_

#include <JPEGDEC.h>

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#include <Seeed_FS.h>
#else
#include <FS.h>
#endif

class JpegClass
{
public:
    int jpegDrawCallback(JPEGDRAW *pDraw)
    {
        if (pDraw->y <= _y_bound)
        {
            if ((pDraw->y + pDraw->iHeight - 1) > _y_bound)
            {
                pDraw->iHeight = _y_bound - pDraw->y + 1;
            }
            if (pDraw->x <= _x_bound)
            {
                if ((pDraw->x + pDraw->iWidth - 1) > _x_bound)
                {
                    int16_t w = pDraw->iWidth;
                    int16_t h = pDraw->iHeight;
                    pDraw->iWidth = _x_bound - pDraw->x + 1;
                    pDraw->iHeight = 1;
                    while (h--)
                    {
                        _jpegDrawCallback(pDraw);
                        pDraw->y++;
                        pDraw->pPixels += w;
                    }

                    return 1;
                }
                else
                {
                    return _jpegDrawCallback(pDraw);
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }

    void draw(
        FS *fs, char *filename, JPEG_DRAW_CALLBACK *jpegDrawCallback, bool useBigEndian,
        int x, int y, int widthLimit, int heightLimit)
    {
        _jpegDrawCallback = jpegDrawCallback;
        _x = x;
        _y = y;
        _x_bound = _x + widthLimit - 1;
        _y_bound = _y + heightLimit - 1;

#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
        File f = fs->open(filename, "r");
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
        File f = fs->open(filename, "r");
#elif defined(ESP32)
        File f = fs->open(filename, "r");
#elif defined(ESP8266)
        File f = fs->open(filename, "r");
#else
        File f = fs->open(filename, FILE_READ);
#endif

        _jpeg.open(f, jpegDrawCallback);

        // scale to fit height
        int _scale;
        int iMaxMCUs;
        float ratio = (float)_jpeg.getHeight() / heightLimit;
        if (ratio <= 1)
        {
            _scale = 0;
            iMaxMCUs = widthLimit / 16;
        }
        else if (ratio <= 2)
        {
            _scale = JPEG_SCALE_HALF;
            iMaxMCUs = widthLimit / 8;
        }
        else if (ratio <= 4)
        {
            _scale = JPEG_SCALE_QUARTER;
            iMaxMCUs = widthLimit / 4;
        }
        else
        {
            _scale = JPEG_SCALE_EIGHTH;
            iMaxMCUs = widthLimit / 2;
        }
        _jpeg.setMaxOutputSize(iMaxMCUs);
        if (useBigEndian)
        {
            _jpeg.setPixelType(RGB565_BIG_ENDIAN);
        }
        _jpeg.decode(x, y, _scale);
        _jpeg.close();
    }

private:
    JPEGDEC _jpeg;
    JPEG_DRAW_CALLBACK *_jpegDrawCallback;
    int _x, _y, _x_bound, _y_bound;
};

#endif // _JPEGCLASS_H_