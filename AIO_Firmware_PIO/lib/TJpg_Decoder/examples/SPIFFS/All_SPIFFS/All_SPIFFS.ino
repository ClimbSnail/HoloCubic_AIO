
// Example for library:
// https://github.com/Bodmer/TJpg_Decoder

// This example is for an ESP8266 or ESP32, it renders all Jpeg files
// found in SPIFFS. The test images are in the sketch "data" folder
// (press Ctrl+K to see it). You can add more images of your own to
// the Data folder.

// You must upload the images to SPIFFS using the ESP8266 or ESP32
// Arduino IDE Sketch Data Upload menu option.

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

// Include SPIFFS
#define FS_NO_GLOBALS
#include <FS.h>
#ifdef ESP32
#include "SPIFFS.h" // ESP32 only
#endif

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>              // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

// Support funtion prototypes
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void loadFile(const char *name);

//====================================================================================
//                                    Setup
//====================================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n Testing TJpg_Decoder library");

  // Initialise SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  // Initialise the TFT
  tft.begin();
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
}

//====================================================================================
//                                    Loop
//====================================================================================
#if defined(ESP32)
void loop()
{
  File root = SPIFFS.open("/");
  while (File file = root.openNextFile()) {
    String strname = file.name();
    strname = "/" + strname;
    // If it is not a directory and filename ends in .jpg then load it
    if (!file.isDirectory() && strname.endsWith(".jpg")) {
      loadFile(strname.c_str());
    }
  }
}
#else   // ESP8266 has different SPIFFS methods
void loop()
{
  fs::Dir directory = SPIFFS.openDir("/");
  while (directory.next()) {
    String strname = directory.fileName();
    strname = "/" + strname;
    // If filename ends in .jpg then load it
    if (strname.endsWith(".jpg")) {
      loadFile(strname.c_str());
    }
  }
}
#endif

//====================================================================================
//                                    tft_output
//====================================================================================
// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

//====================================================================================
//                                    load_file
//====================================================================================

void loadFile(const char *name)
{
  tft.fillScreen(TFT_RED);

  // Time recorded for test purposes
  uint32_t t = millis();

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0, scale;
  TJpgDec.getFsJpgSize(&w, &h, name); // Note name preceded with "/"
  tft.setRotation(w > h ? 1 : 0);

  for (scale = 1; scale <= 8; scale <<= 1) {
    if (w <= tft.width() * scale && h <= tft.height() * scale) break;
  }
  TJpgDec.setJpgScale(scale);

  // Draw the image, top left at 0,0
  TJpgDec.drawFsJpg(0, 0, name);

  // How much time did rendering take
  t = millis() - t;

  char buf[80];
  sprintf(buf, "%s %dx%d 1:%d %u ms", name, w, h, scale, t);
  tft.setCursor(0, tft.height() - 8);
  tft.print(buf);
  Serial.println(buf);
  delay(2000);
}
