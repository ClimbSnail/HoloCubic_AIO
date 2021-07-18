#include "media_player.h"
#include "media_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"
#include <SD.h>
#include "DMADrawer.h"

// #define VIDEO_WIDTH 240L
// #define VIDEO_HEIGHT 240L
// #define RGB565_FILENAME_1 "/movie/240_9fps.rgb"
// #define RGB565_FILENAME_2 "/movie/Bad_Apple_240_9fps.rgb"

#define VIDEO_WIDTH 180L
#define VIDEO_HEIGHT 180L
#define RGB565_FILENAME_1 "/movie/180_9fps.rgb"
#define RGB565_FILENAME_2 "/movie/Bad_Apple_180_9fps.rgb"

// #define VIDEO_WIDTH 120L
// #define VIDEO_HEIGHT 120L
// #define RGB565_FILENAME_1 "/movie/120_9fps.rgb"

#define RGB565_BUFFER_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT * 2)

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises

uint8_t *file_buf = NULL;
File file;
int movie_pos_increate = 0;
int movie_pos = 0;
String movie_filename[2] = {RGB565_FILENAME_1, RGB565_FILENAME_2};

void media_player_init(void)
{
    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.05, 0.5, 0.001, 30};
    set_rgb(&rgb_setting);

    Serial.println("ESP.getFreeHeap()---------> 1");
    Serial.println(ESP.getFreeHeap());
    file_buf = (uint8_t *)malloc(RGB565_BUFFER_SIZE);
    Serial.println(ESP.getFreeHeap());
    // DMADrawer::setup(RGB565_BUFFER_SIZE, 40000000,
    //                  TFT_MOSI, TFT_MISO,
    //                  TFT_SCLK, TFT_CS, TFT_DC);

    Serial.println(F("RGB565 video start"));
    file = SD.open(movie_filename[movie_pos]);
    tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2, (tft->height() - VIDEO_HEIGHT) / 2, VIDEO_WIDTH, VIDEO_HEIGHT);
}

void media_player_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        movie_pos_increate = 1;
    }
    else if (TURN_LEFT == act_info->active)
    {
        movie_pos_increate = -1;
    }

    if (TURN_RIGHT == act_info->active || TURN_LEFT == act_info->active)
    {
        movie_pos = (movie_pos + movie_pos_increate + 2) % 2;
        file.close(); // 尝试关闭文件
        Serial.println(F("RGB565 video start"));
        file = SD.open(movie_filename[movie_pos]);
        tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2, (tft->height() - VIDEO_HEIGHT) / 2, VIDEO_WIDTH, VIDEO_HEIGHT);
    }

    // uint8_t file_buf[RGB565_BUFFER_SIZE];
    if (!file_buf)
    {
        Serial.println(F("buf malloc failed!"));
    }

    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Serial.print("Read from file: ");
    if (file.available())
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
    else
    {
        file.close();
        // Serial.println(F("RGB565 video end"));
        Serial.println(F("RGB565 video start"));
        file = SD.open(movie_filename[movie_pos]);
        tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2, (tft->height() - VIDEO_HEIGHT) / 2, VIDEO_WIDTH, VIDEO_HEIGHT);
    }
    // delay(300);
}

void media_player_exit_callback(void)
{
    file.close(); // 退出时关闭文件
    free(file_buf);
    // DMADrawer::close();

    // 恢复RGB灯  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            1, 32, 255,
                            255, 255, 255,
                            1, 1, 1,
                            0.05, 0.5, 0.001, 30};
    set_rgb(&rgb_setting);
}

void media_player_event_notification(APP_EVENT event)
{
}

APP_OBJ media_app = {"Media", &app_movie, media_player_init,
                     media_player_process, media_player_exit_callback,
                     media_player_event_notification};
