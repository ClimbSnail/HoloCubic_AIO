#include "game_2048.h"
#include "game_2048_gui.h"
#include "game2048_contorller.h"
#include "sys/app_controller.h"
#include "common.h"
#include <esp32-hal-timer.h>

#define G2048_APP_NAME "2048"

void taskOne(void *parameter)
{
    while (1)
    {
        //心跳任务
        lv_tick_inc(5);
        delay(5);
    }
    Serial.println("Ending task 1");
    vTaskDelete(NULL);
}

void taskTwo(void *parameter)
{
    while (1)
    {
        // LVGL任务主函数
        screen.routine();
        delay(5);
    }
    Serial.println("Ending task 2");
    vTaskDelete(NULL);
}

GAME2048 game;

struct Game2048AppRunData
{
    int Normal = 0;       //记录移动的方向
    int BornLocation = 0; //记录新棋子的位置
    int *pBoard;
    int *moveRecord;
    BaseType_t xReturned_task_one = NULL;
    TaskHandle_t xHandle_task_one = NULL;
    BaseType_t xReturned_task_two = NULL;
    TaskHandle_t xHandle_task_two = NULL;
};

static Game2048AppRunData *run_data = NULL;

static int game_2048_init(void)
{
    // 初始化运行时的参数
    game_2048_gui_init();

    randomSeed(analogRead(25));
    // 初始化运行时参数
    run_data = (Game2048AppRunData *)calloc(1, sizeof(Game2048AppRunData));
    game.init();
    run_data->pBoard = game.getBoard();
    run_data->moveRecord = game.getMoveRecord();

    run_data->xReturned_task_one = xTaskCreate(
        taskOne,                      /*任务函数*/
        "TaskOne",                    /*带任务名称的字符串*/
        10000,                        /*堆栈大小，单位为字节*/
        NULL,                         /*作为任务输入传递的参数*/
        1,                            /*任务的优先级*/
        &run_data->xHandle_task_one); /*任务句柄*/

    run_data->xReturned_task_two = xTaskCreate(
        taskTwo,                      /*任务函数*/
        "TaskTwo",                    /*带任务名称的字符串*/
        10000,                        /*堆栈大小，单位为字节*/
        NULL,                         /*作为任务输入传递的参数*/
        1,                            /*任务的优先级*/
        &run_data->xHandle_task_two); /*任务句柄*/
    //刷新棋盘显示
    int new1 = game.addRandom();
    int new2 = game.addRandom();
    showBoard(run_data->pBoard);
    //棋子出生动画
    born(new1);
    born(new2);
    // 防止进入游戏时，误触发了向上
    delay(1000);
}

static void game_2048_process(AppController *sys,
                       const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    // 具体操作
    if (TURN_RIGHT == act_info->active)
    {
        game.moveRight();
        if (game.comparePre() == 0)
        {
            showAnim(run_data->moveRecord, 4, game.addRandom(),
                     run_data->pBoard);
        }
    }
    else if (TURN_LEFT == act_info->active)
    {
        game.moveLeft();
        if (game.comparePre() == 0)
        {
            showAnim(run_data->moveRecord, 3, game.addRandom(),
                     run_data->pBoard);
        }
    }
    else if (UP == act_info->active)
    {
        game.moveUp();
        if (game.comparePre() == 0)
        {
            showAnim(run_data->moveRecord, 1, game.addRandom(),
                     run_data->pBoard);
        }
    }
    else if (DOWN == act_info->active)
    {
        game.moveDown();
        if (game.comparePre() == 0)
        {
            showAnim(run_data->moveRecord, 2, game.addRandom(),
                     run_data->pBoard);
        }
    }

    if (game.judge() == 1)
    {
        //   rgb.setRGB(0, 255, 0);
        Serial.println("you win!");
    }
    else if (game.judge() == 2)
    {
        //   rgb.setRGB(255, 0, 0);
        Serial.println("you lose!");
    }

    // 程序需要时可以适当加延时
    delay(300);
}

static int game_2048_exit_callback(void *param)
{
    // 查杀定时器
    if (run_data->xReturned_task_one == pdPASS)
    {
        vTaskDelete(run_data->xHandle_task_one);
    }
    if (run_data->xReturned_task_two == pdPASS)
    {
        vTaskDelete(run_data->xHandle_task_two);
    }

    game_2048_gui_del();
    // 释放资源
    free(run_data);
    run_data = NULL;
}

static void game_2048_message_handle(const char *from, const char *to,
                              APP_MESSAGE_TYPE type, void *message,
                              void *ext_info)
{
    // 目前事件主要是wifi开关类事件（用于功耗控制）
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
    default:
        break;
    }
}

APP_OBJ game_2048_app = {G2048_APP_NAME, &app_game_2048, "",
                         game_2048_init, game_2048_process,
                         game_2048_exit_callback, game_2048_message_handle};
