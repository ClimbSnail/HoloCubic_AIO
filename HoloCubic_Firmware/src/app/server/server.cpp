#include "server.h"
#include "server_gui.h"
#include "web_setting.h"
#include "../sys/app_contorller.h"
#include "../../network.h"
#include "../../common.h"

WebServer server(80);

boolean web_start = 0; // 标志是否开启web server服务，0为关闭 1为开启

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
    web_start = 1;
}

void stop_web_config()
{
    web_start = 0;
    server.stop();
    server.close();
}

void server_init(void)
{
    server_gui_init();
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

    if (0 == web_start)
    {
        // 如果STA模式连接失败 切换成ap模式
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            g_network.open_ap(AP_SSID);
        }
        display_setting(g_network.get_localIp().c_str(),
                        g_network.get_softAPIP().c_str(),
                        "Domain: holocubic",
                        "WebServer Start", anim_type);
        start_web_config();
    }
    server.handleClient(); // 一定需要放在循环里扫描
}

void server_exit_callback(void)
{
    setting_gui_del();
}

void server_event_notification(APP_EVENT event)
{
}

APP_OBJ server_app = {"Server", &app_server, server_init,
                      server_process, server_exit_callback,
                      server_event_notification};