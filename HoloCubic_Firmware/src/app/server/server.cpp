#include "server.h"
#include "server_gui.h"
#include "web_setting.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"

#define SERVER_REFLUSH_INTERVAL 5000UL // 配置界面重新刷新时间(5s)

WebServer server(80);

struct ServerAppRunData
{
    boolean web_start;                    // 标志是否开启web server服务，0为关闭 1为开启
    boolean req_sent;                     // 标志是否发送wifi请求服务，0为关闭 1为开启
    unsigned long serverReflushPreMillis; // 上一回更新的时间
};

static ServerAppRunData *run_data = NULL;

void start_web_config()
{
    //首页
    server.on("/", HomePage);

    init_page_header();
    init_page_footer();
    server.on("/download", File_Download);
    server.on("/upload", File_Upload);
    server.on("/delete", File_Delete);
    server.on("/delete_result", delete_result);

    server.on("/sys_setting", sys_setting);
    server.on("/rgb_setting", rgb_setting);
    server.on("/weather_setting", weather_setting);
    server.on("/weather_old_setting", weather_old_setting);
    server.on("/bili_setting", bili_setting);
    server.on("/picture_setting", picture_setting);
    server.on("/media_setting", media_setting);
    server.on("/screen_setting", screen_setting);
    server.on(
        "/fupload", HTTP_POST,
        []()
        { server.send(200); },
        handleFileUpload);

    //连接
    server.on("/saveSysConf", saveSysConf);
    server.on("/saveRgbConf", saveRgbConf);
    server.on("/saveWeatherConf", saveWeatherConf);
    server.on("/saveWeatherOldConf", saveWeatherOldConf);
    server.on("/saveBiliConf", saveBiliConf);
    server.on("/savePictureConf", savePictureConf);
    server.on("/saveMediaConf", saveMediaConf);
    server.on("/saveScreenConf", saveScreenConf);

    server.begin();
    // MDNS.addService("http", "tcp", 80);
    Serial.println("HTTP server started");
}

void stop_web_config()
{
    run_data->web_start = 0;
    run_data->req_sent = 0;
    server.stop();
    server.close();
}

static int server_init(void)
{
    server_gui_init();
    // 初始化运行时参数
    run_data = (ServerAppRunData *)malloc(sizeof(ServerAppRunData));
    run_data->web_start = 0;
    run_data->req_sent = 0;
    run_data->serverReflushPreMillis = 0;
}

static void server_process(AppController *sys,
                           const Imu_Action *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        stop_web_config();
        sys->app_exit();
        return;
    }

    if (0 == run_data->web_start && 0 == run_data->req_sent)
    {
        // 预显示
        display_setting(
            "WebServer Start",
            "Domain: holocubic",
            "Wait...", "Wait...",
            // "", "",
            LV_SCR_LOAD_ANIM_NONE);
        // 如果web服务没有开启 且 ap开启的请求没有发送 message这边没有作用（填0）
        sys->send_to(SERVER_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_AP, NULL, NULL);
        run_data->req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == run_data->web_start)
    {
        server.handleClient(); // 一定需要放在循环里扫描
        if (doDelayMillisTime(SERVER_REFLUSH_INTERVAL, &run_data->serverReflushPreMillis, false) == true)
        {
            // 发送wifi维持的心跳
            sys->send_to(SERVER_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_ALIVE, NULL, NULL);

            display_setting(
                "WebServer Start",
                "Domain: holocubic",
                WiFi.localIP().toString().c_str(),
                WiFi.softAPIP().toString().c_str(),
                LV_SCR_LOAD_ANIM_NONE);
        }
    }
}

static int server_exit_callback(void *param)
{
    setting_gui_del();
    // 释放运行时参数
    free(run_data);
    run_data = NULL;
}

static void server_message_handle(const char *from, const char *to,
                                  APP_MESSAGE_TYPE type, void *message,
                                  void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_AP:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_setting(
            "WebServer Start",
            "Domain: holocubic",
            WiFi.localIP().toString().c_str(),
            WiFi.softAPIP().toString().c_str(),
            LV_SCR_LOAD_ANIM_NONE);
        start_web_config();
        run_data->web_start = 1;
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

APP_OBJ server_app = {SERVER_APP_NAME, &app_server, "",
                      server_init, server_process,
                      server_exit_callback, server_message_handle};