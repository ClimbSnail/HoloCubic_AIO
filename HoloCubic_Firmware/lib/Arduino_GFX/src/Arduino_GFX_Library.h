#ifndef _ARDUINO_GFX_LIBRARIES_H_
#define _ARDUINO_GFX_LIBRARIES_H_

#include "Arduino_DataBus.h"
#include "databus/Arduino_ESP32I2S8.h"
#include "databus/Arduino_ESP32PAR8.h"
#include "databus/Arduino_ESP32PAR8Q.h"
#include "databus/Arduino_ESP32PAR16.h"
#include "databus/Arduino_ESP32PAR16Q.h"
#include "databus/Arduino_ESP32SPI.h"
#include "databus/Arduino_ESP8266SPI.h"
#include "databus/Arduino_HWSPI.h"
#include "databus/Arduino_mbedSPI.h"
#include "databus/Arduino_NRFXSPI.h"
#include "databus/Arduino_RPiPicoPAR8.h"
#include "databus/Arduino_RPiPicoPAR16.h"
#include "databus/Arduino_RPiPicoSPI.h"
#include "databus/Arduino_RTLPAR8.h"
#include "databus/Arduino_SWSPI.h"

#include "Arduino_GFX.h" // Core graphics library
#if !defined(LITTLE_FOOT_PRINT)
#include "canvas/Arduino_Canvas.h"
#include "canvas/Arduino_Canvas_Indexed.h"
#include "canvas/Arduino_Canvas_3bit.h"
#include "canvas/Arduino_Canvas_Mono.h"
#include "display/Arduino_ILI9488_3bit.h"
#endif // !defined(LITTLE_FOOT_PRINT)

#include "display/Arduino_GC9A01.h"
#include "display/Arduino_HX8347C.h"
#include "display/Arduino_HX8347D.h"
#include "display/Arduino_HX8352C.h"
#include "display/Arduino_HX8357A.h"
#include "display/Arduino_HX8357B.h"
#include "display/Arduino_ILI9225.h"
#include "display/Arduino_ILI9341.h"
#include "display/Arduino_ILI9342.h"
#include "display/Arduino_ILI9481_18bit.h"
#include "display/Arduino_ILI9486_18bit.h"
#include "display/Arduino_ILI9488_18bit.h"
#include "display/Arduino_JBT6K71.h"
#include "display/Arduino_NT35310.h"
#include "display/Arduino_NT35510.h"
#include "display/Arduino_NT39125.h"
#include "display/Arduino_R61529.h"
#include "display/Arduino_SEPS525.h"
#include "display/Arduino_SSD1283A.h"
#include "display/Arduino_SSD1331.h"
#include "display/Arduino_SSD1351.h"
#include "display/Arduino_ST7735.h"
#include "display/Arduino_ST7789.h"
#include "display/Arduino_ST7796.h"

#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#define DISPLAY_DEV_KIT
#define WIO_TERMINAL
#define TFT_CS LCD_SS_PIN
#define TFT_DC LCD_DC
#define TFT_RST -1
#define TFT_BL LCD_BACKLIGHT
#elif defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#define DISPLAY_DEV_KIT
#define M5STACK_CORE
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_CS 14
#define TFT_DC 27
#define TFT_RST 33
#define TFT_BL 32
#elif defined(ARDUINO_ODROID_ESP32)
#define DISPLAY_DEV_KIT
#define ODROID_GO
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_CS 5
#define TFT_DC 21
#define TFT_RST -1
#define TFT_BL 14
/* TTGO T-Watch */
#elif defined(ARDUINO_T) || defined(ARDUINO_TWATCH_BASE) || defined(ARDUINO_TWATCH_2020_V1) || defined(ARDUINO_TWATCH_2020_V2)
#define DISPLAY_DEV_KIT
#define TTGO_T_WATCH
#define TFT_SCK 18
#define TFT_MOSI 19
#define TFT_MISO -1
#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST -1
#define TFT_BL 12
#elif defined(ARDUINO_ARCH_NRF52840)
#define TFT_SCK 13
#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BL 6
#elif defined(ARDUINO_BLACKPILL_F411CE)
#define TFT_CS 4
#define TFT_DC 3
#define TFT_RST 2
#define TFT_BL 1
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
#define TFT_SCK PIN_SPI0_SCK
#define TFT_MOSI PIN_SPI0_MOSI
#define TFT_MISO PIN_SPI0_MISO
#define TFT_CS 17
#define TFT_DC 27
#define TFT_RST 26
#define TFT_BL 28
#elif defined(ESP32)
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO -1
#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST 33
#define TFT_BL 22
#elif defined(ESP8266)
#define TFT_CS 15
#define TFT_DC 4
#define TFT_RST 2
#define TFT_BL 5
#elif defined(RTL8722DM)
#define TFT_CS 18
#define TFT_DC 17
#define TFT_RST 2
#define TFT_BL 23
#else // default pins for Arduino Nano, Mini, Micro and more
#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BL 6
#endif

Arduino_DataBus *create_default_Arduino_DataBus();
Arduino_GFX *create_default_Arduino_GFX();

#endif // _ARDUINO_GFX_LIBRARIES_H_
