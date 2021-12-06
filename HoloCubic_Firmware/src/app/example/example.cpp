#include "example.h"
#include "example_gui.h"
#include "sys/app_contorller.h"
#include "common.h"

// 动态数据，APP的生命周期结束也需要释放它
struct ExampleAppRunData
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

// 常驻数据，可以不随APP的生命周期而释放或删除
struct ExampleAppForeverData
{
    unsigned int val1;
    unsigned int val2;
    unsigned int val3;
};

// 保存APP运行时的参数信息，理论上关闭APP时推荐在 xxx_exit_callback 中释放掉
static ExampleAppRunData *run_data = NULL;

// 当然你也可以添加恒定在内存中的少量变量（退出时不用释放，实现第二次启动时可以读取）
// 考虑到所有的APP公用内存，尽量减少 forever_data 的数据占用
static ExampleAppForeverData forever_data;

void example_init(void)
{
    // 初始化运行时的参数
    example_gui_init();
    // 初始化运行时参数
    run_data = (ExampleAppRunData *)calloc(1, sizeof(ExampleAppRunData));
    run_data->val1 = 0;
    run_data->val2 = 0;
    run_data->val3 = 0;
    // 使用 forever_data 中的变量，任何函数都可以用
    Serial.print(forever_data.val1);
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

void example_event_notification(APP_EVENT_TYPE type, int event_id)
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

APP_OBJ example_app = {"Example", &app_example, "Author HQ\nVersion 1.8.0\n", example_init,
                        example_process, example_exit_callback,
                        example_event_notification};
