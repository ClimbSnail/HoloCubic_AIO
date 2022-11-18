#include "example.h"
#include "example_gui.h"
#include "sys/app_controller.h"
#include "common.h"

#define EXAMPLE_APP_NAME "Example"

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

static int example_init(AppController *sys)
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

    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile("/example.cfg", (uint8_t *)info);
    // 解析数据
    // 将配置数据保存在文件中（持久化）
    g_flashCfg.writeFile("/example.cfg", "value1=100\nvalue2=200");
    
    return 0;
}

static void example_process(AppController *sys,
                            const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 example_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 程序需要时可以适当加延时
    // delay(300);
}

static void example_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 example_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 也可以移除自身的后台任务，放在本APP可控的地方最合适
    // sys->remove_backgroud_task();

    // 程序需要时可以适当加延时
    // delay(300);
}

static int example_exit_callback(void *param)
{
    // 释放资源
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void example_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    // 目前主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
    }
    break;
    default:
        break;
    }
}

APP_OBJ example_app = {EXAMPLE_APP_NAME, &app_example, "Author HQ\nVersion 2.0.0\n",
                       example_init, example_process, example_background_task,
                       example_exit_callback, example_message_handle};
