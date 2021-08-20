#include "picture.h"
#include "picture_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

static boolean enableTftSwapBytes;
static unsigned long pic_perMillis = 0;          // 图片上一回更新的时间
static unsigned long picRefreshInterval = 10000; // 图片播放的时间间隔(5s)

static File_Info *image_file = NULL; // movie文件夹下的文件指针头
static File_Info *pfile = NULL;      // 指向当前播放的文件节点
static bool images_is_empty = true;  // 标志文件是否为空

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft->height())
        return 0;

    // This function will clip the image block rendering automatically at the TFT boundaries
    tft->pushImage(x, y, w, h, bitmap);

    // This might work instead if you adapt the sketch to use the Adafruit_GFX library
    // tft.drawRGBBitmap(x, y, bitmap, w, h);

    // Return 1 to decode next block
    return 1;
}

void picture_init(void)
{
    photo_gui_init();

    // 保存系统的tft设置参数 用于退出时恢复设置
    enableTftSwapBytes = tft->getSwapBytes();
    tft->setSwapBytes(true); // We need to swap the colour bytes (endianess)

    image_file = tf.listDir(IMAGE_PATH);
    if (NULL != image_file)
    {
        pfile = image_file->next_node;
        if (NULL != pfile)
        {
            Serial.println(pfile->file_name);
            images_is_empty = false;
        }
    }

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);
    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
}

void picture_process(AppController *sys,
                     const Imu_Action *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;
    static int image_pos_increate = 1;

    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_OVER_RIGHT;
        image_pos_increate = 1;
        pic_perMillis = millis() - picRefreshInterval; // 间接强制更新
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_OVER_LEFT;
        image_pos_increate = -1;
        pic_perMillis = millis() - picRefreshInterval; // 间接强制更新
    }

    if (doDelayMillisTime(picRefreshInterval, &pic_perMillis, false) == true)
    {
        if (false == images_is_empty)
        {
            if (NULL == pfile->next_node)
            {
                pfile = image_file->next_node;
            }
            else
            {
                pfile = pfile->next_node;
            }
        }
        char file_name[FILENAME_MAX_LEN] = {0};
        sprintf(file_name, "%s/%s", image_file->file_name, pfile->file_name);
        // Draw the image, top left at 0,0
        Serial.print(F("Decode image: "));
        Serial.println(file_name);
        if (NULL != strstr(file_name, ".jpg"))
        {
            // 直接解码jpg格式的图片
            TJpgDec.drawSdJpg(0, 0, file_name);
        }
        else if (NULL != strstr(file_name, ".bin"))
        {
            // 使用LVGL的bin格式的图片
            display_photo(file_name, anim_type);
        }
    }
    delay(300);
}

void picture_exit_callback(void)
{
    photo_gui_del();
    // 释放文件名链表
    release_file_info(image_file);
    // 恢复此前的驱动参数
    tft->setSwapBytes(enableTftSwapBytes);
}

void picture_event_notification(APP_EVENT event)
{
}

APP_OBJ picture_app = {"Picture", &app_picture, picture_init,
                       picture_process, picture_exit_callback,
                       picture_event_notification};