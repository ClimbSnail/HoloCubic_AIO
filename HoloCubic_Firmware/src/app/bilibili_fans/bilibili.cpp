#include "bilibili.h"
#include "bilibili_gui.h"
#include "sys/app_contorller.h"
#include "../../common.h"
#include "fans.h"

unsigned int fans_num = 0;
unsigned int follow_num = 0;

struct BilibiliAppRunDate
{
    unsigned int refresh_status;
    unsigned long refresh_interval;
    unsigned long refresh_time_millis;
};

static BilibiliAppRunDate *run_data = NULL;

void bilibili_init(void)
{
    bilibili_gui_init();
    // 初始化运行时参数
    run_data = (BilibiliAppRunDate *)malloc(sizeof(BilibiliAppRunDate));
    run_data->refresh_status = 0;
    run_data->refresh_interval = 900000;
    run_data->refresh_time_millis = millis() - run_data->refresh_interval;
}

void bilibili_process(AppController *sys,
                      const Imu_Action *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    if (0 == run_data->refresh_status)
    {
        display_bilibili("bilibili", anim_type, fans_num, follow_num);
        // 以下减少网络请求的压力
        if (doDelayMillisTime(run_data->refresh_interval, &run_data->refresh_time_millis, false))
        {
            sys->req_event(&bilibili_app, APP_EVENT_WIFI_CONN, run_data->refresh_status);
        }
    }
    else
    {
        display_bilibili("bilibili", anim_type, fans_num, follow_num);
    }

    delay(300);
}

void bilibili_exit_callback(void)
{
    bilibili_gui_del();
    free(run_data);
    run_data = NULL;
}

void update_fans_num()
{
    Fans fans;
    fans.init();
    fans.send();
    if (fans.httpCode > 0)
    {
        if (fans.httpCode == HTTP_CODE_OK || fans.httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = fans.httpResponse;
            Serial.println("[HTTP] OK");
            Serial.println(payload);
            int startIndex_1 = payload.indexOf("follower") + 10;
            int endIndex_1 = payload.indexOf('}', startIndex_1);
            int startIndex_2 = payload.indexOf("following") + 11;
            int endIndex_2 = payload.indexOf(',', startIndex_2);
            String res = payload.substring(startIndex_1, endIndex_1);
            fans_num = payload.substring(startIndex_1, endIndex_1).toInt();
            follow_num = payload.substring(startIndex_2, endIndex_2).toInt();
            run_data->refresh_status = 1;
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
}

void bilibili_event_notification(APP_EVENT event, int event_id)
{
    if (event == APP_EVENT_WIFI_CONN)
    {
        Serial.print(millis());
        Serial.println("[SYS] bilibili_event_notification");
        if (0 == run_data->refresh_status)
        {
            update_fans_num();
        }
    }
}

APP_OBJ bilibili_app = {"Bili", &app_bilibili, "", bilibili_init,
                        bilibili_process, bilibili_exit_callback,
                        bilibili_event_notification};

