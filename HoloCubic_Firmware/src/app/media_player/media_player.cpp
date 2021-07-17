#include "media_player.h"
#include "media_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"
#include <SD.h>
#include "DMADrawer.h"

// #define VIDEO_WIDTH 240L
// #define VIDEO_HEIGHT 240L
// #define RGB565_FILENAME "/movie/Bad_Apple_240_9fps.rgb"
// #define RGB565_FILENAME_1 "/movie/240_9fps.rgb"

#define VIDEO_WIDTH 180L
#define VIDEO_HEIGHT 180L
#define RGB565_FILENAME "/movie/Bad_Apple_180_9fps.rgb"
#define RGB565_FILENAME_1 "/movie/180_9fps.rgb"
int pos = 0;

// #define VIDEO_WIDTH 120L
// #define VIDEO_HEIGHT 120L
// #define RGB565_FILENAME "/movie/120_9fps.rgb"

#define RGB565_BUFFER_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT * 2)
#define TFT_BRIGHTNESS 128

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises

uint8_t *file_buf = NULL;

void media_player_init(void)
{
    Serial.println("ESP.getFreeHeap()---------> 1");
    Serial.println(ESP.getFreeHeap());
    file_buf = (uint8_t *)malloc(RGB565_BUFFER_SIZE);
    Serial.println(ESP.getFreeHeap());
    DMADrawer::setup(RGB565_BUFFER_SIZE, 40000000,
                     TFT_MOSI, TFT_MISO,
                     TFT_SCLK, TFT_CS, TFT_DC);
}

void media_player_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    // uint8_t file_buf[RGB565_BUFFER_SIZE];
    if (!file_buf)
    {
        Serial.println(F("buf malloc failed!"));
    }
    Serial.println(F("RGB565 video start"));
    tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2, (tft->height() - VIDEO_HEIGHT) / 2, VIDEO_WIDTH, VIDEO_HEIGHT);
    File file;
    if (pos <= 2)
    {
        pos++;
        file = SD.open(RGB565_FILENAME_1);
    }
    else
    {
        file = SD.open(RGB565_FILENAME);
        pos = 0;
    }
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Serial.print("Read from file: ");
    while (file.available())
    {
        // Read video
        // file_buf = (uint8_t *)DMADrawer::getNextBuffer();
        uint32_t l = file.read(file_buf, RGB565_BUFFER_SIZE);
        // Serial.println(F("---------->\n"));
        // // Play video
        // DMADrawer::draw((240 - VIDEO_WIDTH) / 2,
        //                 (240 - VIDEO_HEIGHT) / 2,
        //                 VIDEO_WIDTH, VIDEO_HEIGHT);
        // Serial.println(F("<----------\n"));
        tft->startWrite();
        // tft->writeBytes(file_buf, l);
        tft->pushColors(file_buf, l);
        tft->endWrite();
    }
    file.close();
    // Serial.println(F("RGB565 video end"));
    // delay(300);
}

void media_player_exit_callback(void)
{
    free(file_buf);
    // DMADrawer::close();
}

void media_player_event_notification(APP_EVENT event)
{
}

APP_OBJ media_app = {"Media", &app_movie, media_player_init,
                     media_player_process, media_player_exit_callback,
                     media_player_event_notification};
