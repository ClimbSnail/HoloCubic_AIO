/*******************************************************************************
 * WiFi Photo Frame
 * This is a simple IoT photo frame sample
 * Please find more details at instructables:
 * https://www.instructables.com/id/Face-Aware-OSD-Photo-Frame/
 *
 * Setup steps:
 * 1. Fill your own SSID_NAME, SSID_PASSWORD, HTTP_HOST, HTTP_PORT and HTTP_PATH_TEMPLATE
 * 2. Change your LCD parameters in Arduino_GFX setting
 ******************************************************************************/

/* WiFi settings */
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"
#define HTTP_HOST "YourServerNameOrIP"
#define HTTP_PORT 8080
#define HTTP_PATH_TEMPLATE "/?w=%d&h=%d"

#define HTTP_TIMEOUT 7000 // in ms, wait a while for server processing

/*******************************************************************************
 * Start of Arduino_GFX setting
 *
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: CS:  9, DC:  8, RST:  7, BL:  6
 * ESP32 various dev board     : CS:  5, DC: 27, RST: 33, BL: 22
 * ESP32-C3 various dev board  : CS:  7, DC:  2, RST:  1, BL:  3
 * ESP32-S2 various dev board  : CS: 34, DC: 35, RST: 33, BL: 21
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48
 * ESP8266 various dev board   : CS: 15, DC:  4, RST:  2, BL:  5
 * Raspberry Pi Pico dev board : CS: 17, DC: 27, RST: 26, BL: 28
 * RTL8720 BW16 old patch core : CS: 18, DC: 17, RST:  2, BL: 23
 * RTL8720_BW16 Official core  : CS:  9, DC:  8, RST:  6, BL:  3
 * RTL8722 dev board           : CS: 18, DC: 17, RST: 22, BL: 23
 * RTL8722_mini dev board      : CS: 12, DC: 14, RST: 15, BL: 13
 * Seeeduino XIAO dev board    : CS:  3, DC:  2, RST:  1, BL:  0
 * Teensy 4.1 dev board        : CS: 39, DC: 41, RST: 40, BL: 22
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

#if defined(ESP32)
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
WiFiMulti wifiMulti;
HTTPClient http;
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;
HTTPClient http;
#elif defined(RTL8722DM)
#include <HttpClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
WiFiClient client;
HttpClient http(client);
#endif

#include "JpegDec.h"
static JpegDec jpegDec;

static unsigned long next_show_millis = 0;

char http_path[1024];

void setup()
{
  Serial.begin(115200);
  // while (!Serial);
  Serial.println("WiFi Photo Frame");

  Serial.println("Init WiFi");
#if defined(ESP32)
  wifiMulti.addAP(SSID_NAME, SSID_PASSWORD);
#elif defined(ESP8266)
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);
#elif defined(RTL8722DM)
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
#endif

  Serial.println("Init display");
  gfx->begin();
  gfx->fillScreen(BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  Serial.println("Init WiFi");
  gfx->println("Init WiFi");
#if defined(RTL8722DM)
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    gfx->print(".");
  }
  Serial.println(" CONNECTED");
  gfx->.println(" CONNECTED");
#endif

  Serial.println("Init HTTP path");
  gfx->println("Init HTTP path");
  // setup http_path query value with LCD dimension
  sprintf(http_path, HTTP_PATH_TEMPLATE, gfx->width(), gfx->height());
  Serial.println(http_path);
  gfx->println(http_path);

#if defined(ESP32)
  Serial.println("Init ESP32 WDT");
  gfx->println("Init ESP32 WDT");
  // set WDT timeout a little bit longer than HTTP timeout
  esp_task_wdt_init((HTTP_TIMEOUT / 1000) + 1, true);
  enableLoopWDT();
#endif
}

void loop()
{
  if (millis() < next_show_millis)
  {
    // Serial.println("Wait for next minute...");
    delay(1000);
  }
  else
  {
#if defined(ESP32)
    if ((wifiMulti.run() == WL_CONNECTED))
    {
#elif defined(ESP8266)
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      WiFiClient client;
#elif defined(RTL8722DM)
    if (WiFi.begin(SSID_NAME, SSID_PASSWORD) == WL_CONNECTED)
    {
      WiFiClient client;
#endif
      next_show_millis = ((millis() / 60000L) + 1) * 60000L; // next minute
      printf("[HTTP] begin...\n");
#if defined(ESP32) || defined(ESP8266)
      http.begin(HTTP_HOST, HTTP_PORT, http_path);
#else
      http.begin(client, HTTP_HOST, HTTP_PORT, http_path);
#endif
      http.setTimeout(HTTP_TIMEOUT);
      printf("[HTTP] GET...\n");
      gfx->printf("[HTTP] GET...\n");
#if defined(ESP32) || defined(ESP8266)
      int httpCode = http.GET();
#elif defined(RTL8722DM)
      http.get(HTTP_HOST, HTTP_PORT, http_path);
      int httpCode = http.responseStatusCode();
      http.skipResponseHeaders();
#endif

      printf("[HTTP] GET... code: %d\n", httpCode);
      gfx->printf("[HTTP] GET... code: %d\n", httpCode);
      // HTTP header has been send and Server response header has been handled
      if (httpCode <= 0)
      {
        // printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      else
      {
        if (httpCode != 200)
        {
          printf("[HTTP] Not OK!\n");
          gfx->printf("[HTTP] Not OK!\n");
          delay(5000);
        }
        else
        {
          // get lenght of document(is - 1 when Server sends no Content - Length header)
#if defined(ESP32) || defined(ESP8266)
          int len = http.getSize();
#elif defined(RTL8722DM)
          int len = http.contentLength();
#endif
          printf("[HTTP] size: %d\n", len);
          gfx->printf("[HTTP] size: %d\n", len);

          if (len <= 0)
          {
            printf("[HTTP] Unknow content size: %d\n", len);
            gfx->printf("[HTTP] Unknow content size: %d\n", len);
          }
          else
          {
            unsigned long start = millis();
            // get tcp stream
#if defined(ESP32) || defined(ESP8266)
            static WiFiClient *http_stream = http.getStreamPtr();
            jpegDec.prepare(http_stream_reader, http_stream);
#else
            jpegDec.prepare(http_stream_reader, &client);
#endif
            jpegDec.decode(JPG_SCALE_NONE, jpegDec.gfx_writer, gfx);
            printf("Time used: %lu\n", millis() - start);
          }
        }
      }
#if defined(ESP32) || defined(ESP8266)
      http.end();
#elif defined(RTL8722DM)
      http.stop();
#endif
    }
  }

#if defined(ESP32)
  // notify WDT still working
  feedLoopWDT();
#elif defined(ESP8266)
  yield();
#endif
}

static size_t http_stream_reader(JpegDec *jpegDec, size_t index, uint8_t *buf, size_t len)
{
  WiFiClient *http_stream = (WiFiClient *)jpegDec->input;
  uint8_t wait = 0;
  if (buf)
  {
    // printf("http_stream_reader: index %d, len: %d\n", index, len);
    size_t a = http_stream->available();
    size_t r = 0;
    while ((r < len) && (wait < 10))
    {
      if (a)
      {
        r += http_stream->readBytes(buf + r, min((len - r), a));
      }
      else
      {
        delay(10);
        wait++;
      }
      a = http_stream->available();
    }
    return r;
  }
  else
  {
    // Serial.printf("[HTTP] skip: %d\n", len);
    size_t l = len;
    while ((l) && (wait < 10))
    {
      if (http_stream->available())
      {
        --l;
        http_stream->read();
      }
      else
      {
        delay(10);
        wait++;
      }
    }
    return len;
  }
}
