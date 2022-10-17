#ifndef _ARDUINO_ILI9806_H_
#define _ARDUINO_ILI9806_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define ILI9806_TFTWIDTH 480  ///< ILI9806 max TFT width
#define ILI9806_TFTHEIGHT 854 ///< ILI9806 max TFT height

#define ILI9806_RST_DELAY 120    ///< delay ms wait for reset finish
#define ILI9806_SLPIN_DELAY 120  ///< delay ms wait for sleep in finish
#define ILI9806_SLPOUT_DELAY 120 ///< delay ms wait for sleep out finish

#define ILI9806_NOP 0x00     ///< No-op register
#define ILI9806_SWRESET 0x01 ///< Software reset register
#define ILI9806_RDDID 0x04   ///< Read display identification information
#define ILI9806_RDDST 0x09   ///< Read Display Status

#define ILI9806_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9806_SLPOUT 0x11 ///< Sleep Out
#define ILI9806_PTLON 0x12  ///< Partial Mode ON
#define ILI9806_NORON 0x13  ///< Normal Display Mode ON

#define ILI9806_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9806_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9806_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9806_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9806_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9806_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9806_INVON 0x21    ///< Display Inversion ON
#define ILI9806_GAMMASET 0x26 ///< Gamma Set
#define ILI9806_DISPOFF 0x28  ///< Display OFF
#define ILI9806_DISPON 0x29   ///< Display ON

#define ILI9806_CASET 0x2A ///< Column Address Set
#define ILI9806_PASET 0x2B ///< Page Address Set
#define ILI9806_RAMWR 0x2C ///< Memory Write
#define ILI9806_RAMRD 0x2E ///< Memory Read

#define ILI9806_PTLAR 0x30    ///< Partial Area
#define ILI9806_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define ILI9806_MADCTL 0x36   ///< Memory Access Control
#define ILI9806_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9806_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9806_FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9806_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9806_FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9806_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9806_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9806_PWCTR1 0xC0 ///< Power Control 1
#define ILI9806_PWCTR2 0xC1 ///< Power Control 2
#define ILI9806_PWCTR3 0xC2 ///< Power Control 3
#define ILI9806_PWCTR4 0xC3 ///< Power Control 4
#define ILI9806_PWCTR5 0xC4 ///< Power Control 5
#define ILI9806_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9806_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9806_RDID1 0xDA ///< Read ID 1
#define ILI9806_RDID2 0xDB ///< Read ID 2
#define ILI9806_RDID3 0xDC ///< Read ID 3
#define ILI9806_RDID4 0xDD ///< Read ID 4

#define ILI9806_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9806_GMCTRN1 0xE1 ///< Negative Gamma Correction
#define ILI9806_PWCTR6 0xFC

#define ILI9806_MADCTL_MY 0x80  ///< Bottom to top
#define ILI9806_MADCTL_MX 0x40  ///< Right to left
#define ILI9806_MADCTL_MV 0x20  ///< Reverse Mode
#define ILI9806_MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define ILI9806_MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define ILI9806_MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define ILI9806_MADCTL_MH 0x04  ///< LCD refresh right to left

class Arduino_ILI9806 : public Arduino_TFT
{
public:
  Arduino_ILI9806(Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0, bool ips = false);

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
