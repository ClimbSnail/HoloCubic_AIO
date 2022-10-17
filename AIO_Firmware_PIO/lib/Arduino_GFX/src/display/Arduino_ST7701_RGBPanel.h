#include "../Arduino_DataBus.h"

#if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

#ifndef _ARDUINO_ST7701_RGBPANEL_H_
#define _ARDUINO_ST7701_RGBPANEL_H_

#include "../Arduino_GFX.h"
#include "../databus/Arduino_ESP32RGBPanel.h"

#define ST7701_TFTWIDTH 480
#define ST7701_TFTHEIGHT 864

class Arduino_ST7701_RGBPanel : public Arduino_GFX
{
public:
  Arduino_ST7701_RGBPanel(Arduino_ESP32RGBPanel *databus, int8_t rst = GFX_NOT_DEFINED, int16_t w = ST7701_TFTWIDTH, int16_t h = ST7701_TFTHEIGHT);

  void begin(int32_t speed = 10000000L) override;
  void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override;
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
  void draw16bitBeRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;

protected:
  uint16_t *_framebuffer;
  Arduino_ESP32RGBPanel *_bus;
  int8_t _rst;

private:
};

#endif // _ARDUINO_ST7701_RGBPANEL_H_

#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)
