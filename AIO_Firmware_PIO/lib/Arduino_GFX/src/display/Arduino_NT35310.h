/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * https://github.com/daumemo/IPS_LCD_NT35310_FT6236_Arduino_eSPI_Test
 * Data Sheet:
 * http://read.pudn.com/downloads648/ebook/2620902/NT35310.pdf
 */
#ifndef _ARDUINO_NT35310_H_
#define _ARDUINO_NT35310_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define NT35310_TFTWIDTH 320  // NT35310 max width
#define NT35310_TFTHEIGHT 480 // NT35310 max height

#define NT35310_RST_DELAY 100     // delay ms wait for reset finish
#define NT35310_SLPIN_DELAY 100    // delay ms wait for sleep in finish
#define NT35310_SLPOUT_DELAY 100 // delay ms wait for sleep out finish

// User Command
#define NT35310_NOP 0x00 // No Operation
#define NT35310_SOFT_RESET 0x01 // Software Reset
#define NT35310_RDID 0x04 // Read Display ID
#define NT35310_RDNUMED 0x05 // Read Number of the Errors on DSI
#define NT35310_GET_POWER_MODE 0x0A // Read Display Power Mode
#define NT35310_GET_ADDRESS_MODE 0x0B // Get the Frame Memory to the Display Panel Read Order
#define NT35310_GET_PIXEL_MODE 0x0C // Read Input Pixel Format
#define NT35310_GET_DISPLAY_MODE 0x0D // Read the Current Display Mode
#define NT35310_GET_SIGNAL_MODE 0x0E // Get Display Module Signaling Mode
#define NT35310_RDDSDR1 0x0F // Read Display Self-Diagnostic Result
#define NT35310_ENTER_SLEEP_MODE 0x10 // Enter the Sleep-In Mode
#define NT35310_EXIT_SLEEP_MODE 0x11 // Exit the Sleep-In Mode
#define NT35310_ENTER_PARTIAL_MODE 0x12 // Partial Display Mode On
#define NT35310_ENTER_NORMAL_MODE 0x13 // Normal Display Mode On
#define NT35310_EXIT_INVERT_MODE 0x20 // Display Inversion Off
#define NT35310_ENTER_INVERT_MODE 0x21 // Display Inversion On
#define NT35310_ALLPOFF 0x22 // All Pixel Off
#define NT35310_ALLPON 0x23 // All Pixel On
#define NT35310_GMASET 0x26 // Gamma Curves Selection
#define NT35310_SET_DISPLAY_OFF 0x28 // Display Off
#define NT35310_SET_DISPLAY_ON 0x29 // Display On
#define NT35310_SET_HORIZONTAL_ADDRESS 0x2A // Set the Column Address
#define NT35310_SET_VERTICAL_ADDRESS 0x2B // Set Page Address
#define NT35310_WRITE_MEMORY_START 0x2C // Memory Write Start Command
#define NT35310_SET_MDDI_RAM_READ_ADDRESS 0x2D // Set the RAM Horizontal and Vertical Address
#define NT35310_READ_MEMORY_START 0x2E // Memory Read Start Command
#define NT35310_SET_PARTIAL_AREA 0x30 // Defines the Partial Display Area
#define NT35310_SCRLAR 0x33 // Set Scroll Area
#define NT35310_SET_TEAR_ON 0x35 // Tearing Effect Line ON
#define NT35310_SET_ADDRESS_MODE 0x36 // Memory Data Access Control
#define NT35310_VSCSAD 0x37 // Vertical Scroll Start Address of RAM
#define NT35310_EXIT_IDLE_MODE 0x38 // Idle Mode Off
#define NT35310_ENTER_IDLE_MODE 0x39 // Idle Mode On
#define NT35310_SET_PIXEL_FORMAT 0x3A // Set the Interface Pixel Format
#define NT35310_RGBCTRL 0x3B // RGB Interface Signal Control
#define NT35310_RAMWRC 0x3C // Memory Write Continuously
#define NT35310_RAMRDC 0x3E // RAM Read Continuously
#define NT35310_SET_TEAR_SCANLINE 0x44 // Set Tear Line
#define NT35310_RDSCL 0x45 // Read Scan Line
#define NT35310_ENTER_DSTB_MODE 0x4F // Enter the Deep Standby Mode
#define NT35310_WRDISBV 0x51 // Write Display Brightness
#define NT35310_RDDISBV 0x52 // Read Display Brightness
#define NT35310_WRCTRLD1 0x53 // Write CTRL Display
#define NT35310_RDCTRLD 0x54 // Read CTRL Display
#define NT35310_WRCTRLD2 0x55 // Write CTRL Display
#define NT35310_RDCABC 0x56 // Read Content Adaptive Brightness Control (CABC) Mode
#define NT35310_RDCABCMB 0x5F // Read CABC Minimum Brightness
#define NT35310_RDDSDR2 0x68 // Read Display Self-Diagnostic Result
#define NT35310_SET_MDDI 0x8F
#define NT35310_RDDDBS 0xA1 // Read DDB Start
#define NT35310_RDDDBC 0xA8 // Read DDB Continue
#define NT35310_RDFCS 0xAA // Read First Checksum
#define NT35310_MDDI_WAKE_TOGGLE 0xAD // MDDI VSYNC BASED LINK WAKE-UP
#define NT35310_STB_EDGE_POSITION 0xAE
#define NT35310_RDCCS 0xAF // Read Continue Checksum
#define NT35310_RDID1 0xDA // Read ID1
#define NT35310_RDID2 0xDB // Read ID2
#define NT35310_RDID3 0xDC // Read ID3
#define NT35310_WRITE_IDLEMODE_BL 0xE1 // Write IDLEMODE_BL_Control
#define NT35310_READ_IDLEMODE_BL 0xE2 // Read IDLEMODE_BL_Control
#define NT35310_PAGE_CTRL 0xED // Unlock CMD2
#define NT35310_PAGE_STATUS 0xFF // PAGE unlock status

// 6.2 CMD2_P0 REGISTER LIST
#define NT35310_DISPLAY_CTRL 0xB0
#define NT35310_PORCH_CTRL 0xB1 // Front & Back Porch Setting
#define NT35310_FRAMERATE_CTRL 0xB2
#define NT35310_SPI_RGB_IF_SETTING 0xB3 // SPI&RGB INTERFACE SETTING
#define NT35310_INVCTRL 0xB4 // Inversion Control
#define NT35310_PMTCTL 0xB5 // Partial and Idle Mode Timing Control
#define NT35310_DISPLAY_CTRL_NORM 0xB6
#define NT35310_DISPLAY_CTRL2 0xB7 // Set the States for LED Control
#define NT35310_MTP_SELECTION 0xB8
#define NT35310_PWR_CTRL1 0xC0
#define NT35310_PWR_CTRL2 0xC1
#define NT35310_PWR_CTRL3 0xC2
#define NT35310_PWR_CTRL5 0xC3
#define NT35310_PWR_CTRL6 0xC4
#define NT35310_PWR_CTRL7 0xC5
#define NT35310_PWR_CTRL8 0xC6
#define NT35310_WID_CTRL1 0xD1 // WID1
#define NT35310_WID_CTRL2 0xD2 // WID2
#define NT35310_WID_CTRL3 0xD3 // WID3
#define NT35310_READID4 0xD4 // Read ID4
#define NT35310_DDB_CTRL 0xD5 // Write DDB Info
#define NT35310_RDVNT 0xDD // Read NV Memory Flag Status
#define NT35310_EPWRITE 0xDE // NV Memory Write Command
#define NT35310_MTPPWR 0xDF // MTP Write function enable
#define NT35310_RDREGEXT1 0xEB // Register read command in SPI interface
#define NT35310_RDREGEXT2 0xEC // Register read command in SPI interface
#define NT35310_PAGE_LOCK1 0xEF // Set the Register to command1
#define NT35310_PAGE_LOCK2 0xBF // Set the Register to command2

// 6.3 CMD2_P1 REGISTER LIST
#define NT35310_3GAMMAR_CTRL_RED_P 0xE0
#define NT35310_3GAMAR_CTRL_RED_N 0xE1
#define NT35310_3GAMMAR_CTRL_GREEN_P 0xE2
#define NT35310_3GAMMAR_CTRL_GREEN_N 0xE3
#define NT35310_3GAMMAR_CTRL_BLUE_P 0xE4
#define NT35310_3GAMMAR_CTRL_BLUE_N 0xE5
#define NT35310_CABC_GAMMA1 0xE6
#define NT35310_CABC_GAMMA2 0xE7
#define NT35310_CABC_GAMMA3 0xE8
#define NT35310_PAGE_LOCK3 0x00 // Set the Register to command2 Page 0

// parameters
#define NT35310_MADCTL_MY 0x80  // Bottom to top
#define NT35310_MADCTL_MX 0x40  // Right to left
#define NT35310_MADCTL_MV 0x20  // Reverse Mode
#define NT35310_MADCTL_ML 0x10  // LCD refresh Bottom to top
#define NT35310_MADCTL_RGB 0x00 // Red-Green-Blue pixel order
#define NT35310_MADCTL_BGR 0x08 // Blue-Green-Red pixel order
#define NT35310_MADCTL_MH 0x04  // LCD refresh right to left

class Arduino_NT35310 : public Arduino_TFT
{
public:
  Arduino_NT35310(
      Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0,
      bool ips = false, int16_t w = NT35310_TFTWIDTH, int16_t h = NT35310_TFTHEIGHT,
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

#endif // _ARDUINO_NT35310_H_
