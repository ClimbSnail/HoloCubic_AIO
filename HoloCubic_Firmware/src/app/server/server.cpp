#include "server.h"
#include "server_gui.h"
#include "web_setting.h"
#include "sys/app_contorller.h"
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
    server.on("/setting", Setting);
    server.on(
        "/fupload", HTTP_POST,
        []()
        { server.send(200); },
        handleFileUpload);

    //连接
    server.on("/saveConf", save_config);

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

void server_init(void)
{
    server_gui_init();
    // 初始化运行时参数
    run_data = (ServerAppRunData *)malloc(sizeof(ServerAppRunData));
    run_data->web_start = 0;
    run_data->req_sent = 0;
    run_data->serverReflushPreMillis = 0;
}

void server_process(AppController *sys,
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
        // 如果web服务没有开启 且 ap开启的请求没有发送 event_id这边没有作用（填0）
        sys->req_event(&server_app, APP_EVENT_WIFI_AP, 0);
        run_data->req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == run_data->web_start)
    {
        server.handleClient(); // 一定需要放在循环里扫描
        if (doDelayMillisTime(SERVER_REFLUSH_INTERVAL, &run_data->serverReflushPreMillis, false) == true)
        {
            // 发送wifi维持的心跳
            sys->req_event(&server_app, APP_EVENT_WIFI_ALIVE, 0);
            
            display_setting(
                "WebServer Start",
                "Domain: holocubic",
                WiFi.localIP().toString().c_str(),
                WiFi.softAPIP().toString().c_str(),
                LV_SCR_LOAD_ANIM_NONE);
        }
    }
}

void server_exit_callback(void)
{
    setting_gui_del();
    // 释放运行时参数
    free(run_data);
    run_data = NULL;
}

void server_event_notification(APP_EVENT_TYPE type, int event_id)
{
    switch (type)
    {
    case APP_EVENT_WIFI_AP:
    {
        Serial.print(F("APP_EVENT_WIFI_AP enable\n"));
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
    case APP_EVENT_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    default:
        break;
    }
}

APP_OBJ server_app = {"WebServer", &app_server, "", server_init,
                      server_process, server_exit_callback,
                      server_event_notification};