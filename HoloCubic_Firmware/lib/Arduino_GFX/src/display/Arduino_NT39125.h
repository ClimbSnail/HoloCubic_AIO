/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * https://github.com/daumemo/IPS_LCD_NT39125_FT6236_Arduino_eSPI_Test
 * Data Sheet:
 * http://read.pudn.com/downloads648/ebook/2620902/NT39125.pdf
 */
#ifndef _Arduino_NT39125_H_
#define _Arduino_NT39125_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define NT39125_TFTWIDTH 240  // NT39125 max width
#define NT39125_TFTHEIGHT 432 // NT39125 max height

#define NT39125_RST_DELAY 200     // delay ms wait for reset finish
#define NT39125_SLPIN_DELAY 20    // delay ms wait for sleep in finish
#define NT39125_SLPOUT_DELAY 1200 // delay ms wait for sleep out finish

// User Command
#define NT39125_NOP 0x00       // NOP
#define NT39125_SWRESET 0x01   // Software Reset
#define NT39125_RDDID 0x04     // Read Display ID
#define NT39125_RDDST 0x09     // Read Display Status
#define NT39125_RDDPM 0x0A     // Read Display Power Mode
#define NT39125_RDDMADCTR 0x0B // Read Display MADCTR
#define NT39125_RDDCOLMOD 0x0C // Read Display Pixel Format
#define NT39125_RDDIM 0x0D     // Read Display Image Mode
#define NT39125_RDDSM 0x0E     // Read Display Signal Mode
#define NT39125_RDDSDR 0x0F    // Read Display Self-Diagnostic Result
#define NT39125_SLPIN 0x10     // Sleep In
#define NT39125_SLPOUT 0x11    // Sleep Out
#define NT39125_PTLON 0x12     // Partial Display Mode On
#define NT39125_NORON 0x13     // Normal Display Mode On
#define NT39125_INVOFF 0x20    // Display Inversion Off
#define NT39125_INVON 0x21     // Display Inversion On
#define NT39125_GAMSET 0x26    // Gamma Set
#define NT39125_DISPOFF 0x28   // Display Off
#define NT39125_DISPON 0x29    // Display On
#define NT39125_CASET 0x2A     // Column Address Set
#define NT39125_RASET 0x2B     // Row Address Set
#define NT39125_RAMWR 0x2C     // Memory Write
#define NT39125_RAMRD 0x2E     // Memory Read
#define NT39125_RGBSET 0x2D    // Color Set for 65k or 4k-Color Display
#define NT39125_PTLAR 0x30     // Partial Area
#define NT39125_SCRLAR 0x33    // Scroll Area
#define NT39125_TEOFF 0x34     // Tearing Effect Line OFF
#define NT39125_TEON 0x35      // Tearing Effect Line ON
#define NT39125_MADCTL 0x36    // Memory Data Access Control
#define NT39125_VSCSAD 0x37    // Vertical Scroll Start Address of RAM
#define NT39125_IDMOFF 0x38    // Idle Mode Off
#define NT39125_IDMON 0x39     // Idle Mode On
#define NT39125_COLMOD 0x3A    // Interface Pixel Format
#define NT39125_RDID1 0xDA     // Read ID1 Value
#define NT39125_RDID2 0xDB     // Read ID2 Value
#define NT39125_RDID3 0xDC     // Read ID3 Value
#define NT39125_IFMODE 0xB0    // Set Display Interface Mode
#define NT39125_FRMCTR1 0xB1   // Set Division ratio for internal clocks of Normal mode
#define NT39125_FRMCTR2 0xB2   // Set Division ratio for internal clocks of Idle mode
#define NT39125_FRMCTR3 0xB3   // Set Division ratio for internal clocks of Partial mode (Idle mode off)
#define NT39125_INVCTR 0xB4    // Inversion Control
#define NT39125_RESCTR 0xB5    // Display Resolution Control
#define NT39125_DISSET5 0xB6   // Display Function set 5
#define NT39125_PWCTR1 0xC0    // Power Control 1
#define NT39125_PWCTR2 0xC1    // Power Control 2
#define NT39125_PWCTR3 0xC2    // Power Control 3 (in Normal mode/ Full colors)
#define NT39125_PWCTR4 0xC3    // Power Control 4 (in Idle mode/ 8-colors)
#define NT39125_PWCTR5 0xC4    // Power Control 5 (in Partial mode/ full-colors)
#define NT39125_VMCTR1 0xC5    // VCOM Control
#define NT39125_VMFCTR 0xC7    // VCOM offset control
#define NT39125_RDVMF 0xC8     // Read VCOM offset value
#define NT39125_WRID2 0xD1     // Write ID2 for MTP program
#define NT39125_WRID3 0xD2     // Write ID3 for MTP program
#define NT39125_RDID4 0xD3     // Read ID4 for IC Vender Code
#define NT39125_RDVNT 0xD4     // Read NV Memory Flag Status
#define NT39125_EPWRITE 0xDE   // MTP write command
#define NT39125_DNVRS 0xDF     // NV Memory Control
#define NT39125_GMCTRP0 0xE0   // Positive RED Gamma Control
#define NT39125_GMCTRN0 0xE1   // Negative RED Gamma Control
#define NT39125_GMCTRP1 0xE2   // Positive GREEN Gamma Control
#define NT39125_GMCTRN1 0xE3   // Negative GREEN Gamma Control
#define NT39125_GMCTRP2 0xE4   // Positive BLUE Gamma Control
#define NT39125_GMCTRN2 0xE5   // Negative BLUE Gamma Control
#define NT39125_GAM_R_SEL 0xEA // Gamma Selection
#define NT39125_TSTGVDD 0xEB   // GVDD output control

// parameters
#define NT39125_MADCTL_MY 0x80  // Bottom to top
#define NT39125_MADCTL_MX 0x40  // Right to left
#define NT39125_MADCTL_MV 0x20  // Reverse Mode
#define NT39125_MADCTL_ML 0x10  // LCD refresh Bottom to top
#define NT39125_MADCTL_RGB 0x00 // Red-Green-Blue pixel order
#define NT39125_MADCTL_BGR 0x08 // Blue-Green-Red pixel order
#define NT39125_MADCTL_MH 0x04  // LCD refresh right to left

class Arduino_NT39125 : public Arduino_TFT
{
public:
  Arduino_NT39125(
      Arduino_DataBus *bus, int8_t rst = -1, uint8_t r = 0,
      bool ips = false, int16_t w = NT39125_TFTWIDTH, int16_t h = NT39125_TFTHEIGHT,
      uint8_t col_offset1 = 0, uint8_t row_offset1 = 0, uint8_t col_offset2 = 0, uint8_t row_offset2 = 0);

  void begin(int32_t speed = 0) override;
  void writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h) override;
  void setRotation(uint8_t r) override;
  void invertDisplay(bool) override;
  void displayOn() override;
  void displayOff() override;

protected:
  void tftInit() override;

private:
};

#endif
