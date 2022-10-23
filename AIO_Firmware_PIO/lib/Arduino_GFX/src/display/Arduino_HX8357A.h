#ifndef _ARDUINO_HX8357A_H_
#define _ARDUINO_HX8357A_H_

#include <Arduino.h>
#include <Print.h>
#include "../Arduino_GFX.h"
#include "../Arduino_TFT.h"

#define HX8357A_TFTWIDTH 320  // HX8357A max TFT width
#define HX8357A_TFTHEIGHT 480 // HX8357A max TFT height

#define HX8357A_RST_DELAY 1200    // delay ms wait for reset finish

#define HX8357A_HIMAX_ID 0x00                         // Himax ID
#define HX8357A_DISPLAY_MODE_CONTROL 0x01             // Display Mode control
#define HX8357A_COLUMN_ADDRESS_START_2 0x02           // Column address start 2
#define HX8357A_COLUMN_ADDRESS_START_1 0x03           // Column address start 1
#define HX8357A_COLUMN_ADDRESS_END_2 0x04             // Column address end 2
#define HX8357A_COLUMN_ADDRESS_END_1 0x05             // Column address end 1
#define HX8357A_ROW_ADDRESS_START_2 0x06              // Row address start 2
#define HX8357A_ROW_ADDRESS_START_1 0x07              // Row address start 1
#define HX8357A_ROW_ADDRESS_END_2 0x08                // Row address end 2
#define HX8357A_ROW_ADDRESS_END_1 0x09                // Row address end 1
#define HX8357A_PARTIAL_ASREA_START_ROW_2 0x0A        // Partial area start row 2
#define HX8357A_PARTIAL_AREA_START_ROW_1 0x0B         // Partial area start row 1
#define HX8357A_PARTIAL_AREA_END_ROW_2 0x0C           // Partial area end row 2
#define HX8357A_PARTIAL_AREA_END_ROW_1 0x0D           // Partial area end row 1
#define HX8357A_VERTICAL_SCROLL_TOP_FIXED_AREA_2 0x0E // Vertical Scroll Top fixed area 2
#define HX8357A_VERTICAL_SCROLL_TOP_FIXED_AREA_1 0x0F // Vertical Scroll Top fixed area 1
#define HX8357A_VERTICAL_SCROLL_HEIGHT_AREA_2 0x10    // Vertical Scroll height area 2
#define HX8357A_VERTICAL_SCROLL_HEIGHT_AREA_1 0x11    // Vertical Scroll height area 1
#define HX8357A_VERTICAL_SCROLL_BUTTON_AREA_2 0x12    // Vertical Scroll Button area 2
#define HX8357A_VERTICAL_SCROLL_BUTTON_AREA_1 0x13    // Vertical Scroll Button area 1
#define HX8357A_VERTICAL_SCROLL_START_ADDRESS_2 0x14  // Vertical Scroll Start address 2
#define HX8357A_VERTICAL_SCROLL_START_ADDRESS_1 0x15  // Vertical Scroll Start address 1
#define HX8357A_MEMORY_ACCESS_CONTROL 0x16            // Memory Access control
#define HX8357A_COLMOD 0x17                           // COLMOD
#define HX8357A_OSC_CONTROL_1 0x18                    // OSC Control 1
#define HX8357A_OSC_CONTROL_2 0x19                    // OSC Control 2
#define HX8357A_POWER_CONTROL_6 0x1A                  // Power Control 6
#define HX8357A_POWER_CONTROL_5 0x1B                  // Power Control 5
#define HX8357A_POWER_CONTROL_4 0x1C                  // Power Control 4
#define HX8357A_POWER_CONTROL_3 0x1D                  // Power Control 3
#define HX8357A_POWER_CONTROL_2 0x1E                  // Power Control 2
#define HX8357A_POWER_CONTROL_1 0x1F                  // Power Control 1
#define HX8357A_SRAM_CONTROL 0x22                     // SRAM Control
#define HX8357A_VCOM_CONTROL_1 0x23                   // VCOM Control 1
#define HX8357A_VCOM_CONTROL_2 0x24                   // VCOM Control 2
#define HX8357A_VCOM_CONTROL_3 0x25                   // VCOM Control 3
#define HX8357A_DISPLAY_CONTROL_1 0x26                // Display Control 1
#define HX8357A_DISPLAY_CONTROL_2 0x27                // Display Control 2
#define HX8357A_DISPLAY_CONTROL_3 0x28                // Display Control 3
#define HX8357A_FRAME_RATE_CONTROL_1 0x29             // Frame Rate control 1
#define HX8357A_FRAME_RATE_CONTROL_2 0x2A             // Frame Rate Control 2
#define HX8357A_FRAME_RATE_CONTROL_3 0x2B             // Frame Rate Control 3
#define HX8357A_FRAME_RATE_CONTROL_4 0x2C             // Frame Rate Control 4
#define HX8357A_CYCLE_CONTROL_2 0x2D                  // Cycle Control 2
#define HX8357A_CYCLE_CONTROL_3 0x2E                  // Cycle Control 3
#define HX8357A_DISPLAY_INVERSION 0x2F                // Display inversion
#define HX8357A_RGB_INTERFACE_CONTROL_1 0x31          // RGB interface control 1
#define HX8357A_RGB_INTERFACE_CONTROL_2 0x32          // RGB interface control 2
#define HX8357A_RGB_INTERFACE_CONTROL_3 0x33          // RGB interface control 3
#define HX8357A_RGB_INTERFACE_CONTROL_4 0x34          // RGB interface control 4
#define HX8357A_PANEL_CHARACTERIC 0x36                // Panel Characteric
#define HX8357A_OTP_CONTROL_1 0x38                    // OTP Control 1
#define HX8357A_OTP_CONTROL_2 0x39                    // OTP Control 2
#define HX8357A_OTP_CONTROL_3 0x3A                    // OTP Control 3
#define HX8357A_OTP_CONTROL_4 0x3B                    // OTP Control 4
#define HX8357A_CABC_CONTROL_1 0x3C                   // CABC Control 1
#define HX8357A_CABC_CONTROL_2 0x3D                   // CABC Control 2
#define HX8357A_CABC_CONTROL_3 0x3E                   // CABC Control 3
#define HX8357A_CABC_CONTROL_4 0x3F                   // CABC Control 4
#define HX8357A_R1_CONTROL_1 0x40                     // r1 Control (1)
#define HX8357A_R1_CONTROL_2 0x41                     // r1 Control (2)
#define HX8357A_R1_CONTROL_3 0x42                     // r1 Control (3)
#define HX8357A_R1_CONTROL_4 0x43                     // r1 Control (4)
#define HX8357A_R1_CONTROL_5 0x44                     // r1 Control (5)
#define HX8357A_R1_CONTROL_6 0x45                     // r1 Control (6)
#define HX8357A_R1_CONTROL_7 0x46                     // r1 Control (7)
#define HX8357A_R1_CONTROL_8 0x47                     // r1 Control (8)
#define HX8357A_R1_CONTROL_9 0x48                     // r1 Control (9)
#define HX8357A_R1_CONTROL_10 0x49                    // r1 Control (10)
#define HX8357A_R1_CONTROL_11 0x4A                    // r1 Control (11)
#define HX8357A_R1_CONTROL_12 0x4B                    // r1 Control (12)
#define HX8357A_R1_CONTROL_13 0x4C                    // r1 Control (13)
#define HX8357A_R1_CONTROL_18 0x50                    // r1 Control (18)
#define HX8357A_R1_CONTROL_19 0x51                    // r1 Control (19)
#define HX8357A_R1_CONTROL_20 0x52                    // r1 Control (20)
#define HX8357A_R1_CONTROL_21 0x53                    // r1 Control (21)
#define HX8357A_R1_CONTROL_22 0x54                    // r1 Control (22)
#define HX8357A_R1_CONTROL_23 0x55                    // r1 Control (23)
#define HX8357A_R1_CONTROL_24 0x56                    // r1 Control (24)
#define HX8357A_R1_CONTROL_25 0x57                    // r1 Control (25)
#define HX8357A_R1_CONTROL_26 0x58                    // r1 Control (26)
#define HX8357A_R1_CONTROL_27 0x59                    // r1 Control (27)
#define HX8357A_R1_CONTROL_28 0x5A                    // r1 Control (28)
#define HX8357A_R1_CONTROL_29 0x5B                    // r1 Control (29)
#define HX8357A_R1_CONTROL_30 0x5C                    // r1 Control (30)
#define HX8357A_R1_CONTROL_35 0x5D                    // r1 Control (35)
#define HX8357A_TE_CONTROL 0x60                       // TE Control
#define HX8357A_ID1 0x61                              // ID1
#define HX8357A_ID2 0x62                              // ID2
#define HX8357A_ID3 0x63                              // ID3
#define HX8357A_ID4 0x64                              // ID4
#define HX8357A_MDDI_CONTROL_4 0x68                   // MDDI Control 4
#define HX8357A_MDDI_CONTROL_5 0x69                   // MDDI Control 5
#define HX8357A_GPIO_CONTROL_1 0x6B                   // GPIO Control 1
#define HX8357A_GPIO_CONTROL_2 0x6C                   // GPIO Control 2
#define HX8357A_GPIO_CONTROL_3 0x6D                   // GPIO Control 3
#define HX8357A_GPIO_CONTROL_4 0x6E                   // GPIO Control 4
#define HX8357A_GPIO_CONTROL_5 0x6F                   // GPIO Control 5
#define HX8357A_SUB_PANEL_CONTROL_1 0x70              // SUB_PANEL Control 1
#define HX8357A_SUB_PANEL_CONTROL_2 0x71              // SUB_PANEL Control 2
#define HX8357A_SUB_PANEL_CONTROL_3 0x72              // SUB_PANEL Control 3
#define HX8357A_SUB_PANEL_CONTROL_4 0x73              // SUB_PANEL Control 4
#define HX8357A_COLUMN_ADDRESS_COUNTER_2 0x80         // Column address counter 2
#define HX8357A_COLUMN_ADDRESS_COUNTER_1 0x81         // Column address counter 1
#define HX8357A_ROW_ADDRESS_COUNTER_2 0x82            // Row address counter 2
#define HX8357A_ROW_ADDRESS_COUNTER_1 0x83            // Row address counter 1
#define HX8357A_SET_VREF 0xE2                         // Set VREF
#define HX8357A_POWER_SAVING_COUNTER_1 0xE4           // Power saving counter 1
#define HX8357A_POWER_SAVING_COUNTER_2 0xE5           // Power saving counter 2
#define HX8357A_POWER_SAVING_COUNTER_3 0xE6           // Power saving counter 3
#define HX8357A_POWER_SAVING_COUNTER_4 0xE7           // Power saving counter 4
#define HX8357A_OPON_CONTROL_1 0xE8                   // OPON Control 1
#define HX8357A_OPON_CONTROL_2 0xE9                   // OPON Control 2
#define HX8357A_CENON_CONTROL 0xF2                    // GENON Control
#define HX8357A_SET_SPI_RRAD_INDEX 0xFD               // Set SPI Rrad Index
#define HX8357A_GET_SPI_INDEX_DATA 0xFE               // Get SPI Index data
#define HX8357A_PAGE_SELECT 0xFF                      // Page select

#define HX8357A_MADCTL_MY 0x80  // PAGE ADDRESS ORDER
#define HX8357A_MADCTL_MX 0x40  // COLUMN ADDRESS ORDER
#define HX8357A_MADCTL_MV 0x20  // PAGE/COLUMN SELECTION
#define HX8357A_MADCTL_BGR 0x08 // RGB-BGR ORDER
#define HX8357A_MADCTL_SS 0x02  // SOURCE OUTPUT ORDER
#define HX8357A_MADCTL_GS 0x01  // GATE OUTPUT ORDER
#define HX8359A_INV_ON 0x20     // INV_ON enable
#define HX8359A_INV_OFF 0x00    // INV_ON disable

class Arduino_HX8357A : public Arduino_TFT
{
public:
  Arduino_HX8357A(Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0, bool ips = false);

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
