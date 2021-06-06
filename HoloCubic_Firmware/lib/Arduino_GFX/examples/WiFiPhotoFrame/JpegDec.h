#ifndef _JPEGDEC_H_
#define _JPEGDEC_H_

#include <Arduino_GFX.h>

#include "lgfx_tjpgd.h"

typedef enum
{
    JPG_SCALE_NONE,
    JPG_SCALE_2X,
    JPG_SCALE_4X,
    JPG_SCALE_8X,
    JPG_SCALE_MAX = JPG_SCALE_8X
} jpg_scale_t;

typedef class JpegDec JpegDec;
typedef size_t (*jpg_reader_cb)(JpegDec *jpegDec, size_t index, uint8_t *buf, size_t len);
typedef bool (*jpg_writer_cb)(JpegDec *jpegDec, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t *data);

class JpegDec
{
public:
    jpg_scale_t scale;
    jpg_reader_cb reader;
    void *input;
    jpg_writer_cb writer;
    void *output;
    size_t index;
    uint16_t width;
    uint16_t height;
    uint16_t output_width;
    uint16_t output_height;

    void prepare(jpg_reader_cb reader, void *input)
    {
        static uint8_t work[3100];

        this->reader = reader;
        this->input = input;
        this->index = 0;

        JRESULT jres = lgfx_jd_prepare(&_decoder, _jpg_read, work, 3100, this);
        if (jres != JDR_OK)
        {
            return;
        }

        width = _decoder.width;
        height = _decoder.height;
    }

    void decode(jpg_scale_t scale, jpg_writer_cb writer, void *output)
    {
        this->scale = scale;
        this->writer = writer;
        this->output = output;

        output_width = width / (1 << (uint8_t)(scale));
        output_height = height / (1 << (uint8_t)(scale));

        //output write
        JRESULT jres = lgfx_jd_decomp(&_decoder, _jpg_write, (uint8_t)scale);
        if (jres != JDR_OK)
        {
            return;
        }
    }

    static size_t buff_reader(JpegDec *jpegDec, size_t index, uint8_t *buf, size_t len)
    {
        // Serial.printf("index: %d, len: %d\n", index, len);
        uint8_t *b = (uint8_t *)jpegDec->input;

        if (buf)
        {
            memcpy(buf, b + index, len);
        }
        return len; // Returns number of bytes read
    }

    static bool gfx_writer(JpegDec *jpegDec, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t *data)
    {
        Arduino_GFX *gfx = (Arduino_GFX *)jpegDec->output;
        if (data)
        {
            // Serial.printf("%d, %d, %d, %d\n", x, y, w, h);
            gfx->draw24bitRGBBitmap(x, y, data, w, h);
        }
        return true; // Continue to decompression
    }

    static bool framebuffer_writer(JpegDec *jpegDec, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t *data)
    {
        uint16_t *framebuffer = (uint16_t *)jpegDec->output;
        // Serial.printf("%d, %d, %d, %d\n", x, y, w, h);
        if (data)
        {
            for (uint16_t i = 0; i < h; ++i)
            {
                for (uint16_t j = 0; j < w; ++j)
                {
                    uint8_t r = *(data++);
                    uint8_t g = *(data++);
                    uint8_t b = *(data++);
                    framebuffer[(y + i) * jpegDec->output_width + x + j] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }
            }
        }
        return true; // Continue to decompression
    }

private:
    lgfxJdec _decoder;

    static uint32_t _jpg_read(lgfxJdec *decoder, uint8_t *buf, uint32_t len)
    {
        JpegDec *jpegDec = (JpegDec *)decoder->device;
        if (len)
        {
            len = jpegDec->reader(jpegDec, jpegDec->index, buf, len);
            jpegDec->index += len;
        }
        return len;
    }

    static uint32_t _jpg_write(lgfxJdec *decoder, void *bitmap, JRECT *rect)
    {
        int16_t x = rect->left;
        int16_t y = rect->top;
        uint16_t w = rect->right + 1 - x;
        uint16_t h = rect->bottom + 1 - y;
        uint8_t *data = (uint8_t *)bitmap;

        JpegDec *jpegDec = (JpegDec *)decoder->device;

        if (jpegDec->writer)
        {
            return jpegDec->writer(jpegDec, x, y, w, h, data);
        }
        return 0;
    }
};

#endif // _JPEGDEC_H_
