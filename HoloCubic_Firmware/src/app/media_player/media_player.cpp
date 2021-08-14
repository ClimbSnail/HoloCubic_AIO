#include "media_player.h"
#include "media_gui.h"
#include "../sys/app_contorller.h"

void media_player_init(void)
{
    // todo
}

void media_player_process(AppController *sys,
                          const Imu_Action *act_info)
{
    // 功能需补充
    sys->app_exit(); // 退出APP
}

void media_player_exit_callback(void)
{
}

void media_player_event_notification(APP_EVENT event)
{
}

APP_OBJ media_app = {"Media", &app_movie, media_player_init,
                     media_player_process, media_player_exit_callback,
                     media_player_event_notification};
