/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * https://github.com/adafruit/Adafruit_ILI9331.git
 */
#ifndef _ARDUINO_ILI9331_H_
#define _ARDUINO_ILI9331_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define ILI9331_TFTWIDTH 240  ///ILI9331 max TFT width
#define ILI9331_TFTHEIGHT 320 ///ILI9331 max TFT height

#define ILI9331_RST_DELAY 150

#define ILI9331_DRVCODER 0x00 ///Driver Code Read
#define ILI9331_DRVOUTCTL 0x01  ///Driver Output Control 1
#define ILI9331_DRIVWVCTL 0x02  ///LCD Driving Control
#define ILI9331_ENTRY_MODE 0x03 ///Entry Mode  
#define ILI9331_DC1 0x07   ///Display Control 1
#define ILI9331_DC2 0x08   ///Display Control 2
#define ILI9331_DC3 0x09   ///Display Control 3
#define ILI9331_DC4 0x0A   ///Display Control 4
#define ILI9331_RGBDIC1 0x0C   ///RGB Display Interface Control 1
#define ILI9331_FMARK 0x0D   ///Frame Marker Position
#define ILI9331_RGDIC2 0x0F   ///RGB Display Interface Control 2

#define ILI9331_PWCTL1 0x10  ///Power Control 1
#define ILI9331_PWCTL2 0x11  ///Power Control 2
#define ILI9331_PWCTL3 0x12  ///Power Control 3
#define ILI9331_PWCTL4 0x13  ///Power Control 4

#define ILI9331_MC 0x20     ///GRAM Horizontal Address Set
#define ILI9331_MP 0x21     ///GRAM Vertical Address Set
#define ILI9331_MW 0x22     ///Write Data to GRAM

#define ILI9331_PWCTL7 0x29     ///Power Control 7
#define ILI9331_FRCR 0x2B     ///Frame Rate and Color Control

#define ILI9331_HSA 0x50     ///Horizontal RAM Address Position
#define ILI9331_HSE 0x51     ///Horizontal RAM Address Position
#define ILI9331_VSA 0x52     ///Vertical RAM Address Position
#define ILI9331_VSE 0x53     ///Vertical RAM Address Position

#define ILI9331_GSC1 0x60     ///Gate Scan Control
#define ILI9331_GSC2 0x61     ///Gate Scan Control
#define ILI9331_GSC3 0x6A     ///Gate Scan Control

#define ILI9331_PANCLT1 0x90     ///Panel Interface Control 1
#define ILI9331_PANCLT2 0x92     ///Panel Interface Control 2
#define ILI9331_PANCLT4 0x95     ///Panel Interface Control 4
#define ILI9331_PANCLT5 0x97     ///Panel Interface Control 5

#define ILI9331_WBRI 0xB1     ///Write Display Brightness Value
#define ILI9331_RBRI 0xB2     ///Read Display Brightness Value
#define ILI9331_WBRICTRL 0xB3     ///Write CTRL Display Value
#define ILI9331_RBRICTRL 0xB4     ///Read CTRL Display Value
#define ILI9331_WCABC 0xB5     ///Write Content Adaptive Brightness Control Value
#define ILI9331_RCABC 0xB6     ///Read Content Adaptive Brightness Control Value
#define ILI9331_WCABCMIN 0xBE     ///Write CABC Minimum Brightness
#define ILI9331_RCABCMIN 0xBF     ///Read CABC Minimum Brightness
#define ILI9331_CABCCTL1 0xC8     ///CABC Control 1
#define ILI9331_CABCCTL2 0xC9     ///CABC Control 2
#define ILI9331_CABCCTL3 0xCA     ///CABC Control 3
#define ILI9331_CABCCTL4 0xCB     ///CABC Control 4
#define ILI9331_CABCCTL5 0xCC     ///CABC Control 5
#define ILI9331_CABCCTL6 0xCD     ///CABC Control 6
#define ILI9331_CABCCTL7 0xCD     ///CABC Control 7

class Arduino_ILI9331 : public Arduino_TFT
{
public:
  Arduino_ILI9331(Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0, bool ips = false);

  void begin(int32_t speed = 0) override;

  void setRotation(uint8_t r) override;

  void writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h) override;

  void invertDisplay(bool) override;
  void displayOn() override;
  void displayOff() override;

protected:
  void tftInit() override;

private:
};

#endif
