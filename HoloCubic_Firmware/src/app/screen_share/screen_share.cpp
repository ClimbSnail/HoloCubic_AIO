#include "screen_share.h"
#include "screen_share_gui.h"
#include "../sys/app_contorller.h"

void screen_share_init(void)
{
    // todo
}

void screen_share_process(AppController *sys,
                          const Imu_Action *act_info)
{
    // 功能需补充
    sys->app_exit(); // 退出APP
}

void screen_exit_callback(void)
{
}

void screen_event_notification(APP_EVENT event)
{
}

APP_OBJ screen_share_app = {"Screen", &app_screen, screen_share_init,
                            screen_share_process, screen_exit_callback,
                            screen_event_notification};