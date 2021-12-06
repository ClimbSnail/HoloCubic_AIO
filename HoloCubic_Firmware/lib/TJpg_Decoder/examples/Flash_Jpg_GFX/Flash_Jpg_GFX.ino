// Adafruit_GFX compatible example for library:
// https://github.com/Bodmer/TJpg_Decoder

// This example renders a Jpeg file that is stored in an array within Flash (program) memory
// see panda.h tab.  The panda image file being ~13Kbytes.

// Include the array
#include "panda.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

//ESP32 example setup
//*
#define TFT_CS  27
#define TFT_DC  26
#define TFT_RST  5
//#define SD_CS   4
//*/

//ESP8266 example setup
/*
#define TFT_CS  D3
#define TFT_DC  D8
#define TFT_RST D4
//#define SD_CS   4
//*/

#define TFT_BLACK       0x0000
#define TFT_RED         0xF800

// Include the TFT library
#include "SPI.h"
#include <Adafruit_ILI9341.h>
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // In ILI9341 library this function clips the image block at TFT boundaries
  tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n Testing TJpg_Decoder library");

  // Initialise the TFT
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
}

void loop()
{
  tft.fillScreen(TFT_RED);

  // Time recorded for test purposes
  uint32_t t = millis();

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, panda, sizeof(panda));
  Serial.print("Width = "); Serial.print(w); Serial.print(", height = "); Serial.println(h);

  // Draw the image, top left at 0,0
  TJpgDec.drawJpg(0, 0, panda, sizeof(panda));

  // How much time did rendering take (ESP8266 80MHz 473ms, 160MHz 266ms, ESP32 SPI 116ms)
  t = millis() - t;
  Serial.print(t); Serial.println(" ms");

  // Wait before drawing again
  delay(2000);
}
