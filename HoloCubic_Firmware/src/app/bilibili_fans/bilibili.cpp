#include "bilibili.h"
#include "bilibili_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"


void bilibili_init(void)
{
}

void bilibili_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    // delay(300);
}

void bilibili_exit_callback(void)
{
}

void bilibili_event_notification(APP_EVENT event)
{
}

APP_OBJ bilibili_app = {"Media", &app_bilibili, bilibili_init,
                     bilibili_process, bilibili_exit_callback,
                     bilibili_event_notification};
