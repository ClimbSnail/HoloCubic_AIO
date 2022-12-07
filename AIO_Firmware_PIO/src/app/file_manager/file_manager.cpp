#include "message.h"
#include "file_manager.h"
#include "file_manager_gui.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include "ESP32FtpServer.h"

#define FILE_MANAGER_APP_NAME "File Manager"

#define FILE_MANAGER_REFLUSH_INTERVAL 2000UL // 配置界面重新刷新时间(2s)

#define RECV_BUFFER_SIZE 2000    //
#define SEND_BUFFER_SIZE 2000    //
#define SERVER_PORT 8081         //设置监听端口
#define SHARE_WIFI_ALIVE 20000UL // 维持wifi心跳的时间（20s）

FtpServer ftpSrv; // 定义FTP服务端

struct FileManagerAppRunData
{
    boolean tcp_start;                    // 标志是否开启web server服务，0为关闭 1为开启
    boolean req_sent;                     // 标志是否发送wifi请求服务，0为关闭 1为开启
    unsigned long serverReflushPreMillis; // 上一回更新的时间
    unsigned long apAlivePreMillis;       // 上一回更新的时间
    uint8_t *recvBuf;                     // 接收数据缓冲区
    uint8_t *sendBuf;                     // 发送数据缓冲区
};

static FileManagerAppRunData *run_data = NULL;
static int file_maneger_init(AppController *sys)
{
    file_maneger_gui_init();
    // 初始化运行时参数
    run_data = (FileManagerAppRunData *)calloc(1, sizeof(FileManagerAppRunData));
    run_data->tcp_start = 0;
    run_data->req_sent = 0;
    run_data->serverReflushPreMillis = 0;
    run_data->recvBuf = (uint8_t *)calloc(1, RECV_BUFFER_SIZE);
    run_data->sendBuf = (uint8_t *)calloc(1, SEND_BUFFER_SIZE);
    return 0;
}

static void file_maneger_process(AppController *sys,
                                 const ImuAction *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        sys->app_exit();
        return;
    }

    if (0 == run_data->tcp_start && 0 == run_data->req_sent)
    {
        // 预显示
        display_file_manager(
            "File Manager",
            WiFi.softAPIP().toString().c_str(),
            "21",
            "Wait connect ....",
            LV_SCR_LOAD_ANIM_NONE);
        // 如果web服务没有开启 且 ap开启的请求没有发送 event_id这边没有作用（填0）
        sys->send_to(FILE_MANAGER_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
        run_data->req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == run_data->tcp_start)
    {
        if (doDelayMillisTime(SHARE_WIFI_ALIVE, &run_data->apAlivePreMillis, false))
        {
            // 发送wifi维持的心跳
            sys->send_to(FILE_MANAGER_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_ALIVE, NULL, NULL);
        }
        ftpSrv.handleFTP(); // make sure in loop you call handleFTP()!!
    }
}

static void file_maneger_background_task(AppController *sys,
                                         const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int file_maneger_exit_callback(void *param)
{
    file_manager_gui_del();

    if (NULL == run_data->recvBuf)
    {
        free(run_data->recvBuf);
        run_data->recvBuf = NULL;
    }

    if (NULL == run_data->sendBuf)
    {
        free(run_data->sendBuf);
        run_data->sendBuf = NULL;
    }

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void file_maneger_message_handle(const char *from, const char *to,
                                        APP_MESSAGE_TYPE type, void *message,
                                        void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_file_manager(
            "File Manager",
            WiFi.localIP().toString().c_str(),
            "21",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
        ftpSrv.begin("holocubic", "aio");
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_file_manager(
            "File Manager",
            WiFi.localIP().toString().c_str(),
            "21",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
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

APP_OBJ file_manager_app = {FILE_MANAGER_APP_NAME, &app_file_manager, "",
                            file_maneger_init, file_maneger_process, file_maneger_background_task,
                            file_maneger_exit_callback, file_maneger_message_handle};
                            