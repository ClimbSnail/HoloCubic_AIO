#include "media_player.h"
#include "media_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"
#include "../../sd_card.h"
#include <SD.h>
#include "DMADrawer.h"

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define RGB565_BUFFER_SIZE 28850 // (57600)
#define MOVIE_PATH "/movie"

#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises

uint8_t *display_buf;
File file;
int movie_pos_increate = 0;
File_Info *movie_file = NULL; // movie文件夹下的文件指针头
File_Info *pfile = NULL;      // 指向当前播放的文件节点
bool movie_is_empty = true;   // 标志文件是否为空

bool start_video(bool create_new)
{
    if (false == movie_is_empty)
    {

        if (true == create_new)
        {
            if (NULL == pfile->next_node)
            {
                pfile = movie_file->next_node;
            }
            else
            {
                pfile = pfile->next_node;
            }
        }

        char file_name[30] = {0};
        sprintf(file_name, "%s/%s", movie_file->file_name, pfile->file_name);
        Serial.print("RGB565 video start --------> ");
        Serial.println(file_name);
        file = SD.open(file_name);
        // tft->initDMA();
        tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2, (tft->height() - VIDEO_HEIGHT) / 2, VIDEO_WIDTH, VIDEO_HEIGHT);
        return true;
    }
    else
    {
        return false;
    }
}

void media_player_init(void)
{
    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.05, 0.5, 0.001, 30};
    set_rgb(&rgb_setting);

    movie_file = tf.listDir(MOVIE_PATH);
    if (NULL != movie_file)
    {
        pfile = movie_file->next_node;
        if (NULL != pfile)
        {
            movie_is_empty = false;
        }
    }

    Serial.print("Stack: ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));

    Serial.print("heap_caps_get_free_size(): ");
    Serial.println((unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT));
    Serial.print("heap_caps_get_largest_free_block() :");
    Serial.println((unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    // Serial.print("heap_caps_dump_all() :");
    // heap_caps_dump_all();

    // Serial.println("ESP.getFreeHeap()--------->");
    // Serial.println(ESP.getFreeHeap());
    display_buf = (uint8_t *)malloc(RGB565_BUFFER_SIZE);
    // Serial.println(ESP.getFreeHeap());
    // Serial.println("<---------ESP.getFreeHeap()");

    // DMADrawer::setup(RGB565_BUFFER_SIZE, 40000000,
    //                  TFT_MOSI, TFT_MISO,
    //                  TFT_SCLK, TFT_CS, TFT_DC);

    Serial.print("heap_caps_get_free_size(): ");
    Serial.println((unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT));
    Serial.print("heap_caps_get_largest_free_block() :");
    Serial.println((unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    // 创建播放
    start_video(false);
}

void media_player_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    
    if (true == movie_is_empty)
    {
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
        file.close(); // 尝试关闭文件

        // 创建播放
        start_video(true);
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
        uint32_t l = 0;
        tft->startWrite();
        // tft->writeBytes(display_buf, l);

        // Serial.println(F("---------->\n"));
        // Play video
        // display_buf = (uint8_t *)DMADrawer::getNextBuffer();
        // l = file.read(display_buf, RGB565_BUFFER_SIZE);
        // Serial.println("DMADrawer::draw");
        // DMADrawer::draw((240 - VIDEO_WIDTH) / 2,
        //                 (240 - VIDEO_HEIGHT) / 2,
        //                 VIDEO_WIDTH, VIDEO_HEIGHT/4);
        // Serial.println(F("<----------\n"));

        l = file.read(display_buf, RGB565_BUFFER_SIZE);
        tft->pushColors(display_buf, l);
        //tft->pushImageDMA();
        l = file.read(display_buf, RGB565_BUFFER_SIZE);
        tft->pushColors(display_buf, l);
        l = file.read(display_buf, RGB565_BUFFER_SIZE);
        tft->pushColors(display_buf, l);
        l = file.read(display_buf, RGB565_BUFFER_SIZE);
        tft->pushColors(display_buf, l);

        tft->endWrite();
    }
    else
    {
        file.close();
        Serial.println(F("RGB565 video end"));
        // 创建播放(重复播放)
        start_video(false);
    }
    // delay(300);
}

void media_player_exit_callback(void)
{
    file.close(); // 退出时关闭文件
    if (NULL != display_buf)
    {
        free(display_buf);
        display_buf = NULL;
    }
    // 释放
    release_file_info(movie_file);
    // DMADrawer::close();
    tft->deInitDMA();

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
