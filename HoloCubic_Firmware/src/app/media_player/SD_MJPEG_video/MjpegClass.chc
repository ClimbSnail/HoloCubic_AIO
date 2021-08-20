#ifndef _MJPEGCLASS_H_
#define _MJPEGCLASS_H_

#pragma GCC optimize("O3")

#define READ_BUFFER_SIZE 2048

#include <esp_heap_caps.h>
#include <FS.h>
#include <Arduino_TFT.h>
#include "tjpgdClass.h"

class MjpegClass
{
public:
  bool setup(File input, uint8_t *mjpeg_buf, Arduino_TFT *tft, bool multiTask)
  {
    _input = input;
    _mjpeg_buf = mjpeg_buf;
    _tft = tft;
    _multiTask = multiTask;

    _tft_width = gfx->width();
    _tft_height = gfx->height();

    if (!_read_buf)
    {
      _read_buf = (uint8_t *)malloc(READ_BUFFER_SIZE);
    }
    for (int i = 0; i < 2; ++i)
    {
      if (!_out_bufs[i])
      {
        _out_bufs[i] = (uint8_t *)heap_caps_malloc(_tft_width * 48 * 2, MALLOC_CAP_DMA);
      }
    }

    _out_buf = _out_bufs[0];

    if (_multiTask)
    {
      _jdec.multitask_begin();
    }

    return true;
  }

  bool readMjpegBuf()
  {
    if (_inputindex == 0)
    {
      _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
      _inputindex += _buf_read;
    }
    _mjpeg_buf_offset = 0;
    int i = 3;
    bool found_FFD9 = false;
    if (_buf_read > 0)
    {
      i = 3;
      while ((_buf_read > 0) && (!found_FFD9))
      {
        if ((_mjpeg_buf_offset > 0) && (_mjpeg_buf[_mjpeg_buf_offset - 1] == 0xFF) && (_read_buf[0] == 0xD9)) // JPEG trailer
        {
          found_FFD9 = true;
        }
        else
        {
          while ((i < _buf_read) && (!found_FFD9))
          {
            if ((_read_buf[i] == 0xFF) && (_read_buf[i + 1] == 0xD9)) // JPEG trailer
            {
              found_FFD9 = true;
              ++i;
            }
            ++i;
          }
        }

        // Serial.printf("i: %d\n", i);
        memcpy(_mjpeg_buf + _mjpeg_buf_offset, _read_buf, i);
        _mjpeg_buf_offset += i;
        size_t o = _buf_read - i;
        if (o > 0)
        {
          // Serial.printf("o: %d\n", o);
          memcpy(_read_buf, _read_buf + i, o);
          _buf_read = _input.read(_read_buf + o, READ_BUFFER_SIZE - o);
          _inputindex += _buf_read;
          _buf_read += o;
          // Serial.printf("_buf_read: %d\n", _buf_read);
        }
        else
        {
          _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
          _inputindex += _buf_read;
        }
        i = 0;
      }
      if (found_FFD9)
      {
        return true;
      }
    }

    return false;
  }

  bool drawJpg()
  {
    _fileindex = 0;
    _remain = _mjpeg_buf_offset;
    TJpgD::JRESULT jres = _jdec.prepare(jpgRead, this);
    if (jres != TJpgD::JDR_OK)
    {
      Serial.printf("prepare failed! %d\r\n", jres);
      return false;
    }

    _out_width = std::min<int32_t>(_jdec.width, _tft_width);
    _jpg_x = (_tft_width - _jdec.width) >> 1;
    if (0 > _jpg_x)
    {
      _off_x = -_jpg_x;
      _jpg_x = 0;
    }
    else
    {
      _off_x = 0;
    }
    _out_height = std::min<int32_t>(_jdec.height, _tft_height);
    _jpg_y = (_tft_height - _jdec.height) >> 1;
    if (0 > _jpg_y)
    {
      _off_y = -_jpg_y;
      _jpg_y = 0;
    }
    else
    {
      _off_y = 0;
    }

    if (_multiTask)
    {
      jres = _jdec.decomp_multitask(jpgWrite16, jpgWriteRow);
    }
    else
    {
      jres = _jdec.decomp(jpgWrite16, jpgWriteRow);
    }

    if (jres != TJpgD::JDR_OK)
    {
      Serial.printf("decomp failed! %d\r\n", jres);
      return false;
    }
    return true;
  }

private:
  File _input;
  uint8_t *_read_buf;
  uint8_t *_mjpeg_buf;
  int32_t _mjpeg_buf_offset = 0;

  Arduino_TFT *_tft;
  bool _multiTask;
  uint8_t *_out_bufs[2];
  uint8_t *_out_buf;
  TJpgD _jdec;

  int32_t _inputindex = 0;
  int32_t _buf_read;
  int32_t _remain = 0;
  uint32_t _fileindex;

  int32_t _tft_width;
  int32_t _tft_height;
  int32_t _out_width;
  int32_t _out_height;
  int32_t _off_x;
  int32_t _off_y;
  int32_t _jpg_x;
  int32_t _jpg_y;

  static uint32_t jpgRead(TJpgD *jdec, uint8_t *buf, uint32_t len)
  {
    MjpegClass *me = (MjpegClass *)jdec->device;
    if (len > me->_remain)
      len = me->_remain;
    if (buf)
    {
      memcpy(buf, (const uint8_t *)me->_mjpeg_buf + me->_fileindex, len);
    }
    me->_fileindex += len;
    me->_remain -= len;
    return len;
  }

  // for 16bit color panel
  static uint32_t jpgWrite16(TJpgD *jdec, void *bitmap, TJpgD::JRECT *rect)
  {
    MjpegClass *me = (MjpegClass *)jdec->device;

    uint16_t *dst = (uint16_t *)me->_out_buf;

    uint_fast16_t x = rect->left;
    uint_fast16_t y = rect->top;
    uint_fast16_t w = rect->right + 1 - x;
    uint_fast16_t h = rect->bottom + 1 - y;
    uint_fast16_t outWidth = me->_out_width;
    uint_fast16_t outHeight = me->_out_height;
    uint8_t *src = (uint8_t *)bitmap;
    uint_fast16_t oL = 0, oR = 0;

    if (rect->right < me->_off_x)
      return 1;
    if (x >= (me->_off_x + outWidth))
      return 1;
    if (rect->bottom < me->_off_y)
      return 1;
    if (y >= (me->_off_y + outHeight))
      return 1;

    if (me->_off_y > y)
    {
      uint_fast16_t linesToSkip = me->_off_y - y;
      src += linesToSkip * w * 3;
      h -= linesToSkip;
    }

    if (me->_off_x > x)
    {
      oL = me->_off_x - x;
    }
    if (rect->right >= (me->_off_x + outWidth))
    {
      oR = (rect->right + 1) - (me->_off_x + outWidth);
    }

    int_fast16_t line = (w - (oL + oR));
    dst += oL + x - me->_off_x;
    src += oL * 3;
    do
    {
      int i = 0;
      do
      {
        uint_fast8_t r8 = src[i * 3 + 0] & 0xF8;
        uint_fast8_t g8 = src[i * 3 + 1];
        uint_fast8_t b5 = src[i * 3 + 2] >> 3;
        r8 |= g8 >> 5;
        g8 &= 0x1C;
        b5 = (g8 << 3) + b5;
        dst[i] = r8 | b5 << 8;
      } while (++i != line);
      dst += outWidth;
      src += w * 3;
    } while (--h);

    return 1;
  }

  static uint32_t jpgWriteRow(TJpgD *jdec, uint32_t y, uint32_t h)
  {
    static int flip = 0;
    MjpegClass *me = (MjpegClass *)jdec->device;
    if (y == 0)
    {
      me->_tft->setAddrWindow(me->_jpg_x, me->_jpg_y, jdec->width, jdec->height);
    }

    me->_tft->startWrite();
    me->_tft->writeBytes((uint8_t *)me->_out_buf, jdec->width * h * 2);
    me->_tft->endWrite();

    flip = !flip;
    me->_out_buf = me->_out_bufs[flip];

    return 1;
  }
};

#endif // _MJPEGCLASS_H_
