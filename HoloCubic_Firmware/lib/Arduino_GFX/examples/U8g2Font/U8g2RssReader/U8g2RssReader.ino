/*******************************************************************************
 * U8g2 RSS Reader
 * This is a simple RSS Reader sample with UTF-8 support
 *
 * Setup steps:
 * 1. Fill your own SSID_NAME, SSID_PASSWORD, RSS_HOST, RSS_PORT, RSS_PATH
 * 2. Change your LCD parameters in Arduino_GFX setting
 ******************************************************************************/

/* WiFi settings */
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"
#define RSS_HOST "rss.weather.gov.hk"
#define RSS_PORT 80
#define RSS_PATH "/rss/LocalWeatherForecast_uc.xml"

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
#include <U8g2lib.h>
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
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
WiFiMulti wifiMulti;
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;
#elif defined(RTL8722DM)
#include <HttpClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#endif

void setup(void)
{
    Serial.begin(115200);

    Serial.println("Arduino RSS Panel");

    // Connect WiFi
#if defined(ESP32)
    wifiMulti.addAP(SSID_NAME, SSID_PASSWORD);
#elif defined(ESP8266)
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);
#elif defined(RTL8722DM)
    WiFi.begin(SSID_NAME, SSID_PASSWORD);
#endif

    gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setUTF8Print(true); // enable UTF8 support for the Arduino print() function

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    /* U8g2 font list: https://github.com/olikraus/u8g2/wiki/fntlistall */
    /* U8g2 Unifont list: https://github.com/olikraus/u8g2/wiki/fntgrpunifont */
    gfx->setFont(u8g2_font_unifont_t_chinese4);
    gfx->setTextColor(WHITE);
}

void loop(void)
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

        printf("[HTTP] begin...\n");
#if defined(ESP32) || defined(ESP8266)
        HTTPClient http;

        if (http.begin(RSS_HOST, RSS_PORT, RSS_PATH))
        {
            printf("[HTTP] GET...\n");
            // start connection and send HTTP header
            int httpCode = http.GET();

            printf("[HTTP] GET... code: %d\n", httpCode);

            // httpCode will be negative on error
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                printf("[HTTP] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    // get length of document (is -1 when Server sends no Content-Length header)
                    int len = http.getSize();
                    printf("[HTTP] size: %d\n", len);

                    // get XML string
                    String xml = http.getString();
#elif defined(RTL8722DM)
        HttpClient http(client);

        if (true)
        {
            printf("[HTTP] GET...\n");
            // start connection and send HTTP header
            int err = http.get(RSS_HOST, RSS_PORT, RSS_PATH);
            if (err == 0)
            {
                if (true)
                {
                    int len = http.contentLength();
                    printf("[HTTP] size: %d\n", len);

                    // get XML string
                    String xml = http.readString();
#endif
                    // update hour
                    int key_idx = xml.indexOf("<item>");
                    key_idx = xml.indexOf("<title>", key_idx + 6);
                    int val_start_idx = key_idx + 7;
                    int val_end_idx = xml.indexOf('<', val_start_idx);
                    int update_hour = xml.substring(val_start_idx + 35, val_start_idx + 37).toInt();
                    String title = xml.substring(val_start_idx, val_end_idx);
                    Serial.println(title);

                    gfx->fillScreen(BLACK);
                    gfx->setCursor(0, 16);

                    // gfx->setTextSize(2);
                    gfx->setTextColor(GREEN);
                    gfx->println(title);
                    gfx->println();

                    // description
                    key_idx = xml.indexOf("<description><![CDATA[", val_end_idx);
                    val_start_idx = key_idx + 22;
                    val_end_idx = xml.indexOf("]]></description>", val_start_idx);
                    String description = xml.substring(val_start_idx, val_end_idx);
                    description.trim();
                    Serial.println(description);

                    // gfx->setTextSize(1);
                    gfx->setTextColor(WHITE);
                    val_start_idx = 0;
                    while (val_start_idx < description.length())
                    {
                        val_end_idx = description.indexOf("<br/>", val_start_idx);
                        if (val_end_idx < 0)
                        {
                            val_end_idx = description.length();
                        }
                        String paragraph = description.substring(val_start_idx, val_end_idx);
                        paragraph.trim();
                        gfx->println(paragraph);
                        val_start_idx = val_end_idx + 5;
                    }
                }
            }
#if defined(ESP32) || defined(ESP8266)
            else
            {
                printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
#elif defined(RTL8722DM)
            http.stop();
#endif
        }
        else
        {
            printf("[HTTP] Unable to connect\n");
        }

        delay(60 * 60 * 1000);
    }

    // delay(1000);
}
