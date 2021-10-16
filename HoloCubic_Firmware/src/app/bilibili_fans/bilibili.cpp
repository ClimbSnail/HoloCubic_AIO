#include "bilibili.h"
#include "bilibili_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"

struct BilibiliAppRunDate
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

static BilibiliAppRunDate *run_data = NULL;

void bilibili_init(void)
{
    // 初始化运行时参数
    run_data = (BilibiliAppRunDate *)malloc(sizeof(BilibiliAppRunDate));
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

void bilibili_event_notification(APP_EVENT event, int event_id)
{
}

APP_OBJ bilibili_app = {"Bili", &app_bilibili, bilibili_init,
                        bilibili_process, bilibili_exit_callback,
                        bilibili_event_notification};
