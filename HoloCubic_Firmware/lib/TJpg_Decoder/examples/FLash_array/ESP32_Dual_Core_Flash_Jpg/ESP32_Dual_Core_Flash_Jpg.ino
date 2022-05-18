// Example for library:
// https://github.com/Bodmer/TJpg_Decoder

// This example is for an ESP32, it renders a Jpeg file that is stored in an array within Flash (program) memory
// see panda.h tab.  The panda image file being ~13Kbytes. The Jpeg decoding is done by one processor core 0 and
// the rendering to TFT by processor 1 (which is normally used to execute the Arduino sketches).

// Single processor core Flash_Jpg sketch on ILI9341 at 40MHz SPI renders the panda Jpeg in 103ms
// The Jpeg image takes 66ms to deocde and 37ms to render to TFT
// This sketch uses both ESP32 processors in parallel so decoding and rendering only takes 66ms
// Processor 1 runs the main sketch, processor 0 runs the Jpeg decoder

// Include the array
#include "panda.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>               // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();          // Invoke custom library

// Global variables available to BOTH processors 0 and 1
TaskHandle_t Task1;
const uint8_t* arrayName;           // Name of FLASH array containing Jpeg
bool doDecoding = false;            // Mutex flag to start decoding
bool mcuReady = false;              // Mutex flag to indicate an MCU block is ready for rendering
uint16_t mcuBuffer[16*16];          // Buffer to grab a snapshot of decoded MCU block
int32_t mcu_x, mcu_y, mcu_w, mcu_h; // Snapshot of the place to render the MCU

// This next function will be called by the TJpg_Decoder library during decoding of the jpeg file
// A copy of the decoded MCU block is grabbed for rendering so decoding can then continue while
// the MCU block is rendered on the TFT. Note: This function is called by processor 0
bool mcu_decoded(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  while(mcuReady) yield(); // Wait here if rendering of last MCU block to TFT is still in progress

  memcpy(mcuBuffer, bitmap, 16*16*2); // Grab a copy of the MCU block image
  mcu_x = x;                          // Grab postion and size of MCU block
  mcu_y = y;
  mcu_w = w;
  mcu_h = h;
  mcuReady = true; // Flag to tell processor 1 that rendering of MCU can start

  // Return 1 to decode next Jpeg MCU block
  return 1;
}

// This is the task that runs on processor 0 (Arduino sketch runs on processor 1)
// It decodes the Jpeg image
void decodeJpg(void* p) {
  // This is an infinite loop, effectively the same as the normal sketch loop()
  // but this function and loop is running on processor 0
  for(;;) {
    // Decode the Jpeg image
    if (doDecoding) { // Only start decoding if main sketch sets this flag
      TJpgDec.drawJpg(0, 0, arrayName, sizeof(panda)); // Runs until complete image decoded
      doDecoding = false; // Set mutex false to indicate decoding has ended
    }
    // Must yield in this loop
    yield();
  }
}

void setup()
{
  //Create task decodeJpg to run on processor 0 to decode a Jpeg
  xTaskCreatePinnedToCore(decodeJpg, "decodeJpg", 10000, NULL, 0, NULL, 0);

  Serial.begin(115200);
  Serial.println("\n\n Testing TJpg_Decoder library");

  // Initialise the TFT
  tft.begin();
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the mcu buffer function above
  TJpgDec.setCallback(mcu_decoded);
}

void loop()
{
  tft.fillScreen(TFT_RED);

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, panda, sizeof(panda));
  Serial.print("Width = "); Serial.print(w); Serial.print(", height = "); Serial.println(h);

  // Time recorded for test purposes
  uint32_t t = millis();

  // The order here is important, doDecoding must be set "true" last after other parameters have been defined
  arrayName  = panda; // Name of FLASH array to be decoded
  mcuReady   = false; // Flag which is set true when a MCU block is ready for display
  doDecoding = true;  // Flag to tell task to decode the image

  // Only render MCU blocks if decoding is in progress OR an MCU is ready to render
  // Note: the OR mcuReady is required so the last block is rendered after decoding has ended
  while(doDecoding || mcuReady) {
    if (mcuReady) {
      tft.pushImage(mcu_x, mcu_y, mcu_w, mcu_h, mcuBuffer);
      mcuReady = false;
    }
    // Must yield in this loop
    yield();
  }

  // How much time did rendering take
  t = millis() - t;
  Serial.print(t); Serial.println(" ms");

  // Wait before drawing again
  delay(2000);
}
