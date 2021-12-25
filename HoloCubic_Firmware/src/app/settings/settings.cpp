#include "settings.h"
#include "settings_gui.h"
#include "sys/app_contorller.h"
#include "common.h"

struct SettingsAppRunData
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

static SettingsAppRunData *run_data = NULL;

void settings_init(void)
{
    // 初始化运行时的参数
    settings_gui_init();

    display_settings(AIO_VERSION, "v 2.0.0", LV_SCR_LOAD_ANIM_NONE);

    // 初始化运行时参数
    run_data = (SettingsAppRunData *)calloc(1, sizeof(SettingsAppRunData));
    run_data->val1 = 0;
    run_data->val2 = 0;
    run_data->val3 = 0;

}

void settings_process(AppController *sys,
                      const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    // 发送请求，当请求完成后自动会调用 settings_event_notification 函数
    // sys->req_event(&settings_app, APP_EVENT_WIFI_CONN, run_data->val1);
    // 程序需要时可以适当加延时
    delay(200);
}

void settings_exit_callback(void)
{
    settings_gui_del();
    // 释放资源
    free(run_data);
    run_data = NULL;
}

void settings_event_notification(APP_EVENT_TYPE type, int event_id)
{
    // 目前事件主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_EVENT_WIFI_CONN:
    {
        // todo
    }
    break;
    case APP_EVENT_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_EVENT_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    default:
        break;
    }
}

APP_OBJ settings_app = {"Settings", &app_settings, "", settings_init,
                        settings_process, settings_exit_callback,
                        settings_event_notification};
