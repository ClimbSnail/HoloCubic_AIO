#include "picture.h"
#include "picture_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"

int image_pos = 0;                       // 记录相册播放的历史
unsigned long pic_perMillis = 0;         // 图片上一回更新的时间
unsigned long picRefreshInterval = 5000; // 图片播放的时间间隔(5s)

void picture_init(void)
{
    photo_gui_init();
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
        // photo_file_num为了防止当 image_pos_increate为-1时，数据取模出错
        image_pos = (image_pos + image_pos_increate + photo_file_num) % photo_file_num;
        display_photo(file_name_list[image_pos], anim_type);
    }
    delay(300);
}

void picture_exit_callback(void)
{
    photo_gui_del();
}

void picture_event_notification(APP_EVENT event)
{
}

APP_OBJ picture_app = {"Picture", &app_picture, picture_init,
                       picture_process, picture_exit_callback,
                       picture_event_notification};