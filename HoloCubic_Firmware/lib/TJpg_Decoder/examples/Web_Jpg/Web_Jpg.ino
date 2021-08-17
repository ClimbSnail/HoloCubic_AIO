// Example for library:
// https://github.com/Bodmer/TJpg_Decoder

// This example is for an ESP8266 or ESP32, it fetches a Jpeg file
// from the web and saves it in a SPIFFS file. You must have SPIFFS
// space allocated in the IDE.

// Chenge next 2 lines to suit your WiFi network
#define WIFI_SSID "Your_SSID"
#define PASSWORD "Your password"


// Include the jpeg decoder library
#include <TJpg_Decoder.h>

// Include SPIFFS
#define FS_NO_GLOBALS
#include <FS.h>

// Include WiFi and http client
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#else
  #include "SPIFFS.h" // Required for ESP32 only
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

// Load tabs attached to this sketch
#include "List_SPIFFS.h"
#include "Web_Fetch.h"

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>              // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

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
  tft.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  WiFi.begin(WIFI_SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();

  // This is for demoe purposes only so that file is fetched each time this is run
  if (SPIFFS.exists("/M81.jpg") == true) {
    Serial.println("For test only, removing file");
    SPIFFS.remove("/M81.jpg");
    //SPIFSS.remove("/F35.jpg");
  }
}

void loop()
{
  // List files stored in SPIFFS
  listSPIFFS();

  // Time recorded for test purposes
  uint32_t t = millis();

  // Fetch the jpg file from the specified URL, examples only, from imgur
  bool loaded_ok = getFile("http://i.imgur.com/C77RWcq.jpg", "/M81.jpg"); // Note name preceded with "/"
  //bool loaded_ok = downloadFile("http://i.imgur.com/OnW2qOO.jpg", "/F35.jpg");

  t = millis() - t;
  if (loaded_ok) { Serial.print(t); Serial.println(" ms to download"); }

  // List files stored in SPIFFS, should have the file now
  listSPIFFS();

  t = millis();

  // Now draw the SPIFFS file
  TJpgDec.drawFsJpg(0, 0, "/M81.jpg");
  //TJpgDec.drawFsJpg(0, 0, "/F35.jpg");

  t = millis() - t;
  Serial.print(t); Serial.println(" ms to draw to TFT");

  // Wait forever
  while(1) yield();
}
