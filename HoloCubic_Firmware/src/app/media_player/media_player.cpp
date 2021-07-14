#include "media_player.h"
#include "media_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"
#include <SD.h>

// #define VIDEO_WIDTH 240L
// #define VIDEO_HEIGHT 240L
// #define RGB565_FILENAME "/movie/240_9fps.rgb"
#define VIDEO_WIDTH 180L
#define VIDEO_HEIGHT 180L
#define RGB565_FILENAME "/movie/180_9fps.rgb"
#define RGB565_BUFFER_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT * 2)
#define TFT_BRIGHTNESS 128

uint8_t *file_buf = NULL;

void media_player_init(void)
{
    // todo
    file_buf = (uint8_t *)malloc(RGB565_BUFFER_SIZE);
}

void media_player_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    // uint8_t file_buf[14400];
    if (!file_buf)
    {
        Serial.println(F("buf malloc failed!"));
    }
    Serial.println(F("RGB565 video start"));
    tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2, (tft->height() - VIDEO_HEIGHT) / 2, VIDEO_WIDTH, VIDEO_HEIGHT);

    File file = SD.open(RGB565_FILENAME);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Serial.print("Read from file: ");
    while (file.available())
    {
        // Read video
        uint32_t l = file.read(file_buf, RGB565_BUFFER_SIZE);

        // Play video
        tft->startWrite();
        tft->writeBytes(file_buf, l);
        // tft->writePixels((uint16_t *)file_buf, l>>2);
        tft->endWrite();
    }
    file.close();
    // Serial.println(F("RGB565 video end"));
    // delay(300);
}

void media_player_exit_callback(void)
{
    free(file_buf);
}

void media_player_event_notification(APP_EVENT event)
{
}

APP_OBJ media_app = {"Media", &app_movie, media_player_init,
                     media_player_process, media_player_exit_callback,
                     media_player_event_notification};
