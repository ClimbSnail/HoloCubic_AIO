#include "game_snake.h"
#include "game_snake_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "freertos/semphr.h"

// 游戏名称
#define GAME_APP_NAME "Snake"

#define SNAKE_SPEED 1000

struct SnakeAppRunData
{
    unsigned int score;
    int gameStatus;
    BaseType_t xReturned_task_run = pdFALSE;
    TaskHandle_t xHandle_task_run = NULL;
};

static SnakeAppRunData *run_data = NULL;

void taskRun(void *parameter)
{
    while (1)
    {
        // LVGL任务主函数，处理所有的LVGL任务，包括绘制界面，处理用户输入等。
        AIO_LVGL_OPERATE_LOCK(lv_task_handler();)
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    Serial.println("Ending lv_task_handler");
    vTaskDelete(NULL);
}

static int game_snake_init(AppController *sys)
{
    // 随机数种子
    randomSeed(analogRead(A0));
    // 初始化运行时的参数
    game_snake_gui_init();
    // 初始化运行时参数
    run_data = (SnakeAppRunData *)calloc(1, sizeof(SnakeAppRunData));
    run_data->score = 0;
    run_data->gameStatus = 0;
    run_data->xReturned_task_run = xTaskCreate(
        taskRun,                      /*任务函数*/
        "taskRun",                    /*任务名称*/
        8 * 1024,                     /*堆栈大小，单位为字节*/
        NULL,                         /*参数*/
        1,                            /*优先级*/
        &run_data->xHandle_task_run); /*任务句柄*/

    return 0;
}

static void game_snake_process(AppController *sys, const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        run_data->gameStatus = -1;
        sys->app_exit(); // 退出APP
        return;
    }

    // 操作触发
    if (TURN_RIGHT == act_info->active)
    {
        update_driection(DIR_RIGHT);
    }
    else if (TURN_LEFT == act_info->active)
    {
        update_driection(DIR_LEFT);
    }
    else if (UP == act_info->active)
    {
        update_driection(DIR_UP);
    }
    else if (DOWN == act_info->active)
    {
        update_driection(DIR_DOWN);
    }

    if (run_data->gameStatus == 0 && run_data->xReturned_task_run == pdPASS)
    {
        AIO_LVGL_OPERATE_LOCK(display_snake(run_data->gameStatus, LV_SCR_LOAD_ANIM_NONE););
    }

    // 速度控制
    delay(SNAKE_SPEED);
}

static void game_snake_background_task(AppController *sys,
                                       const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 game_snake_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 也可以移除自身的后台任务，放在本APP可控的地方最合适
    // sys->remove_backgroud_task();

    // 程序需要时可以适当加延时
    // delay(300);
}

static int game_snake_exit_callback(void *param)
{
    // 查杀任务
    if (run_data->xReturned_task_run == pdPASS)
    {
        vTaskDelete(run_data->xHandle_task_run);
    }

    // 释放lvgl_mutex信号量
    xSemaphoreGive(lvgl_mutex);

    // 释放页面资源
    game_snake_gui_del();

    // 释放事件资源
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void game_snake_message_handle(const char *from, const char *to,
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
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
    }
    break;
    default:
        break;
    }
}

APP_OBJ game_snake_app = {GAME_APP_NAME, &app_game_snake, "",
                          game_snake_init, game_snake_process, game_snake_background_task,
                          game_snake_exit_callback, game_snake_message_handle};
