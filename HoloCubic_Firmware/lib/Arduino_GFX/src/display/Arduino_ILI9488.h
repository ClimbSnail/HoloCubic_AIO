/*
 * start rewrite from:
 * https://github.com/nopnop2002/esp-idf-parallel-tft
 */
#ifndef _ARDUINO_ILI9488_H_
#define _ARDUINO_ILI9488_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define ILI9488_TFTWIDTH 320  ///< ILI9488 max TFT width
#define ILI9488_TFTHEIGHT 480 ///< ILI9488 max TFT height

#define ILI9488_RST_DELAY 150    ///< delay ms wait for reset finish
#define ILI9488_SLPIN_DELAY 150  ///< delay ms wait for sleep in finish
#define ILI9488_SLPOUT_DELAY 150 ///< delay ms wait for sleep out finish

// Generic commands used by ILI9488_eSPI.cpp
#define ILI9488_NOP 0x00
#define ILI9488_SWRESET 0x01

#define ILI9488_SLPIN 0x10
#define ILI9488_SLPOUT 0x11

#define ILI9488_INVOFF 0x20
#define ILI9488_INVON 0x21

#define ILI9488_DISPOFF 0x28
#define ILI9488_DISPON 0x29

#define ILI9488_CASET 0x2A
#define ILI9488_PASET 0x2B
#define ILI9488_RAMWR 0x2C

#define ILI9488_RAMRD 0x2E

#define ILI9488_MADCTL 0x36

#define ILI9488_MADCTL_MY 0x80
#define ILI9488_MADCTL_MX 0x40
#define ILI9488_MADCTL_MV 0x20
#define ILI9488_MADCTL_ML 0x10
#define ILI9488_MADCTL_RGB 0x00
#define ILI9488_MADCTL_BGR 0x08
#define ILI9488_MADCTL_MH 0x04
#define ILI9488_MADCTL_SS 0x02
#define ILI9488_MADCTL_GS 0x01

class Arduino_ILI9488 : public Arduino_TFT
{
public:
  Arduino_ILI9488(Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0, bool ips = false);

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

#endif // #ifndef _ARDUINO_ILI9488_H_
