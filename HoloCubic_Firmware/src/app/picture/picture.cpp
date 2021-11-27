#include "picture.h"
#include "picture_gui.h"
#include "sys/app_contorller.h"
#include "common.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

struct PictureAppRunData
{
    unsigned long pic_perMillis;      // 图片上一回更新的时间
    unsigned long picRefreshInterval; // 图片播放的时间间隔(10s)

    File_Info *image_file;      // movie文件夹下的文件指针头
    File_Info *pfile;           // 指向当前播放的文件节点
    int image_pos_increate = 1; // 文件的遍历方向
    bool tftSwapStatus;
};

static PictureAppRunData *run_data = NULL;

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

static File_Info *get_next_file(File_Info *p_cur_file, int direction)
{
    // 得到 p_cur_file 的下一个 类型为FILE_TYPE_FILE 的文件（即下一个非文件夹文件）
    if (NULL == p_cur_file)
    {
        return NULL;
    }

    File_Info *pfile = direction == 1 ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file)
    {
        if (FILE_TYPE_FILE == pfile->file_type)
        {
            break;
        }
        pfile = direction == 1 ? pfile->next_node : pfile->front_node;
    }
    return pfile;
}

void picture_init(void)
{
    photo_gui_init();
    // 初始化运行时参数
    run_data = (PictureAppRunData *)malloc(sizeof(PictureAppRunData));
    run_data->pic_perMillis = 0;
    run_data->picRefreshInterval = 10000;
    run_data->image_file = NULL;
    run_data->pfile = NULL;
    run_data->image_pos_increate = 1;
    // 保存系统的tft设置参数 用于退出时恢复设置
    run_data->tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true); // We need to swap the colour bytes (endianess)

    run_data->image_file = tf.listDir(IMAGE_PATH);
    if (NULL != run_data->image_file)
    {
        run_data->pfile = get_next_file(run_data->image_file->next_node, 1);
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

    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_OVER_RIGHT;
        run_data->image_pos_increate = 1;
        run_data->pic_perMillis = millis() - run_data->picRefreshInterval; // 间接强制更新
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_OVER_LEFT;
        run_data->image_pos_increate = -1;
        run_data->pic_perMillis = millis() - run_data->picRefreshInterval; // 间接强制更新
    }

    if (NULL == run_data->image_file)
    {
        sys->app_exit();
        return;
    }

    if (doDelayMillisTime(run_data->picRefreshInterval, &run_data->pic_perMillis, false) == true)
    {
        if (NULL != run_data->image_file)
        {
            run_data->pfile = get_next_file(run_data->pfile, run_data->image_pos_increate);
        }
        char file_name[PIC_FILENAME_MAX_LEN] = {0};
        snprintf(file_name, PIC_FILENAME_MAX_LEN, "%s/%s", run_data->image_file->file_name, run_data->pfile->file_name);
        // Draw the image, top left at 0,0
        Serial.print(F("Decode image: "));
        Serial.println(file_name);
        if (NULL != strstr(file_name, ".jpg") || NULL != strstr(file_name, ".JPG"))
        {
            // 直接解码jpg格式的图片
            TJpgDec.drawSdJpg(0, 0, file_name);
        }
        else if (NULL != strstr(file_name, ".bin") || NULL != strstr(file_name, ".BIN"))
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
    release_file_info(run_data->image_file);
    // 恢复此前的驱动参数
    tft->setSwapBytes(run_data->tftSwapStatus);

    // 释放运行数据
    free(run_data);
    run_data = NULL;
}

void picture_event_notification(APP_EVENT event, int event_id)
{
}

APP_OBJ picture_app = {"Picture", &app_picture, "", picture_init,
                       picture_process, picture_exit_callback,
                       picture_event_notification};