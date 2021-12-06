/*******************************************************************************
 * PNG Image Viewer
 * This is a simple PNG image viewer example
 * Image Source: https://github.com/logos
 *
 * Dependent libraries:
 * Pngle: https://github.com/kikuchan/pngle.git
 * 
 * Setup steps:
 * 1. Change your LCD parameters in Arduino_GFX setting
 * 2. Upload PNG file
 *   FFat (ESP32):
 *     upload FFat (FatFS) data with ESP32 Sketch Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *   LittleFS (ESP32 / ESP8266 / Pico):
 *     upload LittleFS data with ESP8266 LittleFS Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *     ESP8266: https://github.com/earlephilhower/arduino-esp8266littlefs-plugin
 *     Pico: https://github.com/earlephilhower/arduino-pico-littlefs-plugin.git
 *   SPIFFS (ESP32):
 *     upload SPIFFS data with ESP32 Sketch Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *   SD:
 *     Most Arduino system built-in support SD file system.
 *     Wio Terminal require extra dependant Libraries:
 *     - Seeed_Arduino_FS: https://github.com/Seeed-Studio/Seeed_Arduino_FS.git
 *     - Seeed_Arduino_SFUD: https://github.com/Seeed-Studio/Seeed_Arduino_SFUD.git
 ******************************************************************************/
#define PNG_FILENAME "/octocat.png"

/*******************************************************************************
 * Start of Arduino_GFX setting
 * 
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * ESP32 various dev board     : TFT_CS:  5, TFT_DC: 27, TFT_RST: 33, TFT_BL: 22
 * ESP8266 various dev board   : TFT_CS: 15, TFT_DC:  4, TFT_RST:  2, TFT_BL:  5
 * Raspberry Pi Pico dev board : TFT_CS: 17, TFT_DC: 27, TFT_RST: 26, TFT_BL: 28
 * RTL872x various dev board   : TFT_CS: 18, TFT_DC: 17, TFT_RST:  2, TFT_BL: 23
 * Arduino Nano, Micro and more: TFT_CS:  9, TFT_DC:  8, TFT_RST:  7, TFT_BL:  6
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 3 /* rotation */, false /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#include <Seeed_FS.h>
#include <SD/Seeed_SD.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
#include <LittleFS.h>
#include <SD.h>
#elif defined(ESP32)
#include <FFat.h>
#include <LittleFS.h>
#include <SPIFFS.h>
#include <SD.h>
#elif defined(ESP8266)
#include <LittleFS.h>
#include <SD.h>
#else
#include <SD.h>
#endif

#include <pngle.h>
int16_t xOffset = 0;
int16_t yOffset = 0;

// Pngle init callback
void pngleInitCallback(pngle_t *pngle, uint32_t w, uint32_t h)
{
  int16_t gfxW = gfx->width();
  int16_t gfxH = gfx->height();
  xOffset = (w > gfxW) ? 0 : ((gfxW - w) / 2);
  yOffset = (h > gfxH) ? 0 : ((gfxH - h) / 2);
}

// Pngle draw callback
void pngleDrawCallback(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
  if (rgba[3]) // not transparent
  {
    gfx->fillRect(x + xOffset, y + yOffset, w, h, gfx->color565(rgba[0], rgba[1], rgba[2]));
  }
}

void setup()
{
  Serial.begin(115200);
  // while (!Serial);
  Serial.println("PNG Image Viewer");

  // Init Display
  gfx->begin();
  gfx->fillScreen(BLACK);

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  if (!LittleFS.begin())
  // if (!SD.begin(SS))
#elif defined(ESP32)
  if (!FFat.begin())
  // if (!LittleFS.begin())
  // if (!SPIFFS.begin())
  // if (!SD.begin(SS))
#elif defined(ESP8266)
  if (!LittleFS.begin())
  // if (!SD.begin(SS))
#else
  if (!SD.begin())
#endif
  {
    Serial.println(F("ERROR: File System Mount Failed!"));
    gfx->println(F("ERROR: File System Mount Failed!"));
  }
  else
  {
    unsigned long start = millis();

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
    File pngFile = SD.open(PNG_FILENAME, "r");
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
    File pngFile = LittleFS.open(PNG_FILENAME, "r");
    // File pngFile = SD.open(PNG_FILENAME, "r");
#elif defined(ESP32)
    File pngFile = FFat.open(PNG_FILENAME, "r");
    // File pngFile = LittleFS.open(PNG_FILENAME, "r");
    // File pngFile = SPIFFS.open(PNG_FILENAME, "r");
    // File pngFile = SD.open(PNG_FILENAME, "r");
#elif defined(ESP8266)
    File pngFile = LittleFS.open(PNG_FILENAME, "r");
    // File pngFile = SD.open(PNG_FILENAME, "r");
#else
    File pngFile = SD.open(PNG_FILENAME, FILE_READ);
#endif

    if (!pngFile || pngFile.isDirectory())
    {
      Serial.println(F("ERROR: Failed to open " PNG_FILENAME " file for reading"));
      gfx->println(F("ERROR: Failed to open " PNG_FILENAME " file for reading"));
    }
    else
    {
      pngle_t *pngle = pngle_new();
      pngle_set_init_callback(pngle, pngleInitCallback);
      pngle_set_draw_callback(pngle, pngleDrawCallback);
      char buf[16]; // buffer minimum size is 16 but it can be much larger, e.g. 2048
      int remain = 0;
      int len;
      gfx->fillScreen(PINK); // transprant background color
      while ((len = pngFile.readBytes(buf + remain, sizeof(buf) - remain)) > 0)
      {
        int fed = pngle_feed(pngle, buf, remain + len);
        if (fed < 0)
        {
          Serial.printf("ERROR: %s\n", pngle_error(pngle));
          break;
        }

        remain = remain + len - fed;
        if (remain > 0)
        {
          memmove(buf, buf + fed, remain);
        }
      }

      pngle_destroy(pngle);
      pngFile.close();

      Serial.printf("Time used: %lu\n", millis() - start);
    }
  }
}

void loop()
{
}
