#include "display.h"
#include "ambient.h"
#include "lv_port_indev.h"
#include "lv_port_fatfs.h"
#include "common.h"
#include "config.h"

#include "app/sys/app_contorller.h"
#include "app/media_player/media_player.h"
#include "app/picture/picture.h"
#include "app/screen_share/screen_share.h"
#include "app/server/server.h"
#include "app/bilibili_fans/bilibili.h"
#include "app/weather/weather.h"
#include "app/clock/clock.h"

/*** Component objects **7*/
Display screen;
IMU mpu;
Ambient ambLight;
Imu_Action *act_info;          // 存放mpu6050返回的数据
AppController *app_contorller; // APP控制器

void wifi_auto_process(){
    if (doDelayMillisTime(g_network.m_wifiClickInterval, &g_network.m_preWifiClickMillis, false) == true){
        if (CONN_SUCC != g_network.end_conn_wifi()){
        }
    }
}

void setup(){
    Serial.begin(115200);

    /*** Init screen ***/
    screen.init();
    screen.setBackLight(1.0);

    /*** Init IMU as input device ***/
    lv_port_indev_init();
    app_contorller = new AppController(); // APP控制器
    mpu.init();

    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.1).setRGB(0, 122, 204);

    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();
    lv_fs_if_init();
    config_read("/wifi.txt", &g_cfg);
    tf.listDir("/image", 250);

    // app_contorller->app_register(&weather_app);
    // app_contorller->app_register(&picture_app);
    // app_contorller->app_register(&media_app);
    app_contorller->app_register(&bilibili_app);
    app_contorller->app_register(&clock_app);
    // app_contorller->app_register(&screen_share_app);
    // app_contorller->app_register(&server_app);

    /*** Read WiFi info in SD-Card, then scan & connect WiFi ***/
    g_network.init(g_cfg.ssid, g_cfg.password);

    act_info = mpu.update(200);
}

void loop(){
    screen.routine();
    app_contorller->main_process(act_info); // 运行当前进程
    act_info = mpu.update(200);

    // rgb.setBrightness(ambLight.getLux() / 500.0);
    wifi_auto_process(); // 任务调度
    // malloc(2);
}