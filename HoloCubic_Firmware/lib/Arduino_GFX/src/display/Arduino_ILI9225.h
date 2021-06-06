/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#ifndef _ARDUINO_ILI9225_H_
#define _ARDUINO_ILI9225_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define ILI9225_TFTWIDTH 176  ///< ILI9225 max TFT width
#define ILI9225_TFTHEIGHT 220 ///< ILI9225 max TFT height

#define ILI9225_RST_DELAY 150    ///< delay ms wait for reset finish
#define ILI9225_SLPIN_DELAY 150  ///< delay ms wait for sleep in finish
#define ILI9225_SLPOUT_DELAY 150 ///< delay ms wait for sleep out finish

#define ILI9225_DRIVER_OUTPUT_CTRL 0x01      // Driver Output Control
#define ILI9225_LCD_AC_DRIVING_CTRL 0x02     // LCD AC Driving Control
#define ILI9225_ENTRY_MODE 0x03              // Entry Mode
#define ILI9225_DISP_CTRL1 0x07              // Display Control 1
#define ILI9225_BLANK_PERIOD_CTRL1 0x08      // Blank Period Control
#define ILI9225_FRAME_CYCLE_CTRL 0x0B        // Frame Cycle Control
#define ILI9225_INTERFACE_CTRL 0x0C          // Interface Control
#define ILI9225_OSC_CTRL 0x0F                // Osc Control
#define ILI9225_POWER_CTRL1 0x10             // Power Control 1
#define ILI9225_POWER_CTRL2 0x11             // Power Control 2
#define ILI9225_POWER_CTRL3 0x12             // Power Control 3
#define ILI9225_POWER_CTRL4 0x13             // Power Control 4
#define ILI9225_POWER_CTRL5 0x14             // Power Control 5
#define ILI9225_VCI_RECYCLING 0x15           // VCI Recycling
#define ILI9225_RAM_ADDR_SET1 0x20           // Horizontal GRAM Address Set
#define ILI9225_RAM_ADDR_SET2 0x21           // Vertical GRAM Address Set
#define ILI9225_GRAM_DATA_REG 0x22           // GRAM Data Register
#define ILI9225_GATE_SCAN_CTRL 0x30          // Gate Scan Control Register
#define ILI9225_VERTICAL_SCROLL_CTRL1 0x31   // Vertical Scroll Control 1 Register
#define ILI9225_VERTICAL_SCROLL_CTRL2 0x32   // Vertical Scroll Control 2 Register
#define ILI9225_VERTICAL_SCROLL_CTRL3 0x33   // Vertical Scroll Control 3 Register
#define ILI9225_PARTIAL_DRIVING_POS1 0x34    // Partial Driving Position 1 Register
#define ILI9225_PARTIAL_DRIVING_POS2 0x35    // Partial Driving Position 2 Register
#define ILI9225_HORIZONTAL_WINDOW_ADDR1 0x36 // Horizontal Address Start Position
#define ILI9225_HORIZONTAL_WINDOW_ADDR2 0x37 // Horizontal Address End Position
#define ILI9225_VERTICAL_WINDOW_ADDR1 0x38   // Vertical Address Start Position
#define ILI9225_VERTICAL_WINDOW_ADDR2 0x39   // Vertical Address End Position
#define ILI9225_GAMMA_CTRL1 0x50             // Gamma Control 1
#define ILI9225_GAMMA_CTRL2 0x51             // Gamma Control 2
#define ILI9225_GAMMA_CTRL3 0x52             // Gamma Control 3
#define ILI9225_GAMMA_CTRL4 0x53             // Gamma Control 4
#define ILI9225_GAMMA_CTRL5 0x54             // Gamma Control 5
#define ILI9225_GAMMA_CTRL6 0x55             // Gamma Control 6
#define ILI9225_GAMMA_CTRL7 0x56             // Gamma Control 7
#define ILI9225_GAMMA_CTRL8 0x57             // Gamma Control 8
#define ILI9225_GAMMA_CTRL9 0x58             // Gamma Control 9
#define ILI9225_GAMMA_CTRL10 0x59            // Gamma Control 10

class Arduino_ILI9225 : public Arduino_TFT
{
public:
  Arduino_ILI9225(Arduino_DataBus *bus, int8_t rst = -1, uint8_t r = 0);

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
