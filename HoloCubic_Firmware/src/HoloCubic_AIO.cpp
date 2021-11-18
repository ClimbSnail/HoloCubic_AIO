/***************************************************
  HoloCubic多功能固件源码

  聚合多种APP，内置天气、时钟、相册、特效动画、视频播放、视频投影、
  浏览器文件修改。（各APP具体使用参考说明书）

  Github repositories：https://github.com/ClimbSnail/HoloCubic_AIO

  Last review/edit by ClimbSnail: 2021/08/21
 ****************************************************/

#include "driver/lv_port_indev.h"
#include "driver/lv_port_fatfs.h"
#include "common.h"

#include "sys/app_contorller.h"

#include "app/weather/weather.h"
#include "app/picture/picture.h"
#include "app/media_player/media_player.h"
#include "app/screen_share/screen_share.h"
#include "app/bilibili_fans/bilibili.h"
#include "app/server/server.h"
#include "app/idea_anim/idea.h"
#include "app/settings/settings.h"
#include "app/file_manager/file_manager.h"
#include "app/game_2048/game_2048.h"

/*** Component objects **7*/
IMU mpu;
Imu_Action *act_info;          // 存放mpu6050返回的数据
AppController *app_contorller; // APP控制器

void setup()
{
    Serial.begin(115200);

    Serial.println(F("\nAIO (All in one) version " AIO_VERSION "\n"));

    config_read(NULL, &g_cfg);
    g_time_utile.init();

    /*** Init screen ***/
    screen.init();

    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.05).setRGB(0, 64, 64);

    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();
    lv_fs_if_init();

    app_contorller = new AppController(); // APP控制器
    app_contorller->app_register(&weather_app);
    app_contorller->app_register(&picture_app);
    app_contorller->app_register(&media_app);
    app_contorller->app_register(&screen_share_app);
    app_contorller->app_register(&server_app);
    app_contorller->app_register(&idea_app);
    app_contorller->app_register(&bilibili_app);
    app_contorller->app_register(&settings_app);
    app_contorller->app_register(&game_2048_app);
    app_contorller->app_register(&file_manager_app);

    // 优先显示屏幕 加快视觉上的开机时间
    app_contorller->main_process(&mpu.action_info);

    /*** Init IMU as input device ***/
    lv_port_indev_init();
    mpu.init(); // 初始化比较耗时

    /*** 以此作为MPU6050初始化完成的标志 ***/
    // 初始化RGB灯 HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            1, 32, 255,
                            255, 255, 255,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    // 初始化RGB任务
    rgb_thread_init(&rgb_setting);
}

void loop()
{
    screen.routine();
    act_info = mpu.update(200);
    app_contorller->main_process(act_info); // 运行当前进程
    // Serial.println(ambLight.getLux() / 50.0);
    // rgb.setBrightness(ambLight.getLux() / 500.0);
}