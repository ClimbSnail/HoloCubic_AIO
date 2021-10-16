#include "example.h"
#include "example_gui.h"
#include "../sys/app_contorller.h"
#include "../../common.h"

struct ExampleAppRunDate
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

static ExampleAppRunDate *run_data = NULL;

void example_init(void)
{
    // 初始化运行时的参数
    example_gui_init();
    // 初始化运行时参数
    run_data = (ExampleAppRunDate *)calloc(1, sizeof(ExampleAppRunDate));
    run_data->val1 = 0;
    run_data->val2 = 0;
    run_data->val3 = 0;

}

void example_process(AppController *sys,
                      const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    // 发送请求，当请求完成后自动会调用 example_event_notification 函数
    // sys->req_event(&example_app, APP_EVENT_WIFI_CONN, run_data->val1);
    
    // 程序需要时可以适当加延时
    // delay(300);
}

void example_exit_callback(void)
{
    // 释放资源
    free(run_data);
    run_data = NULL;
}

void example_event_notification(APP_EVENT event, int event_id)
{
    // 目前事件主要是wifi开关类事件（用于功耗控制）
    switch (event)
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

APP_OBJ example_app = {"Example", &app_example, example_init,
                        example_process, example_exit_callback,
                        example_event_notification};
