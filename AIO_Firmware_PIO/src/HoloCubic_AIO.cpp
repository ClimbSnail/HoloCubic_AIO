/***************************************************
  HoloCubic多功能固件源码
  （项目中若参考本工程源码，请注明参考来源）

  聚合多种APP，内置天气、时钟、相册、特效动画、视频播放、视频投影、
  浏览器文件修改。（各APP具体使用参考说明书）

  Github repositories：https://github.com/ClimbSnail/HoloCubic_AIO

  Last review/edit by ClimbSnail: 2023/01/14
 ****************************************************/

#include "driver/lv_port_indev.h"
#include "driver/lv_port_fs.h"

#include "common.h"
#include "sys/app_controller.h"

#include "app/app_conf.h"

#include <SPIFFS.h>
#include <esp32-hal.h>
#include <esp32-hal-timer.h>

bool isCheckAction = false;

/*** Component objects **7*/
ImuAction *act_info;           // 存放mpu6050返回的数据
AppController *app_controller; // APP控制器

TaskHandle_t handleTaskLvgl;

void TaskLvglUpdate(void *parameter)
{
    // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for (;;)
    {
        AIO_LVGL_OPERATE_LOCK(lv_timer_handler();)
        vTaskDelay(5);
    }
}

TimerHandle_t xTimerAction = NULL;
void actionCheckHandle(TimerHandle_t xTimer)
{
    // 标志需要检测动作
    isCheckAction = true;
}

void my_print(const char *buf)
{
    Serial.printf("%s", buf);
    Serial.flush();
}

bool is_night_mode_time(uint8_t current_hour, 
                                    uint8_t night_start, 
                                    uint8_t night_end)
{
    // 处理相同时间的情况（如0-0表示不启用或全天启用，根据需求决定）
    if (night_start == night_end) {
        // 可以根据需求调整：
        // return true;    // 相同时间表示全天启用夜间模式
        return false;      // 相同时间表示不启用夜间模式
    }
    
    // 正常情况：开始时间 < 结束时间（如18:00-23:00）
    if (night_start < night_end) {
        return (current_hour >= night_start && current_hour < night_end);
    }
    // 跨越午夜的情况：开始时间 > 结束时间（如22:00-06:00）
    else {
        return (current_hour >= night_start || current_hour < night_end);
    }
}

// NTP服务器配置
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;  // 中国时区 UTC+8
const int daylightOffset_sec = 0;      // 不使用夏令时

// 初始化时间服务
bool initTime() {
    app_controller->send_to(SERVER_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // 等待时间同步
    Serial.print("等待时间同步");
    struct tm timeinfo;
    for (int i = 0; i < 20; i++) {
        if (getLocalTime(&timeinfo)) {
            Serial.println("\n时间同步成功!");
            Serial.print("当前时间: ");
            Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
            return true;
        }
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n时间同步失败!");
    return false;
}

// 获取当前小时 (0-23)
// 获取完整时间（小时和分钟）
bool get_current_time(uint8_t* hour, uint8_t* minute) {

    struct tm timeinfo;
    if(getLocalTime(&timeinfo)){
        *hour = timeinfo.tm_hour;
        *minute = timeinfo.tm_min;
        return true;
    }
    return false;
}

// 改进的时间判断函数
bool is_night_mode_time_complete(uint8_t current_hour, uint8_t current_minute,
                                 uint8_t night_start_hour, uint8_t night_start_minute,
                                 uint8_t night_end_hour, uint8_t night_end_minute) {
    
    // 转换为分钟数
    uint16_t current_time_minutes = current_hour * 60 + current_minute;
    uint16_t night_start_minutes = night_start_hour * 60 + night_start_minute;
    uint16_t night_end_minutes = night_end_hour * 60 + night_end_minute;
    
    if (night_start_minutes == night_end_minutes) {
        return false; // 不启用夜间模式
    }
    
    if (night_start_minutes < night_end_minutes) {
        return (current_time_minutes >= night_start_minutes && 
                current_time_minutes < night_end_minutes);
    } else {
        return (current_time_minutes >= night_start_minutes || 
                current_time_minutes < night_end_minutes);
    }
}


// 全局变量
bool currentNightMode = false;


void checkAndUpdateBrightness() {
    static bool lastNightMode = false;
    Serial.print("Checking...");
    uint8_t current_hour, current_minute;
    if (!get_current_time(&current_hour, &current_minute)) return; // 时间获取失败
    Serial.print("Current Time: ");
    Serial.print(current_hour);
    RgbConfig *rgb_cfg = &app_controller->rgb_cfg;
    
    bool isNightNow = is_night_mode_time(
        current_hour, 
        rgb_cfg->brightness_night_mode_start,
        rgb_cfg->brightness_night_mode_end
    );
    
    // 如果模式发生变化
    if (isNightNow != lastNightMode) {
        lastNightMode = isNightNow;
        currentNightMode = isNightNow;
        
        if (isNightNow) {
            Serial.println("切换到夜间模式");
            // 设置RGB夜间亮度
            //rgb.setBrightness(rgb_cfg->brightness_night_mode_specified / 100.0);
            // 设置屏幕夜间亮度（需要看你的屏幕API）
            screen.setBackLight(rgb_cfg->brightness_night_mode_specified / 100.0);
        } else {
            Serial.println("切换到日间模式");
            // 设置RGB日间亮度
            //rgb.setBrightness(app_controller->sys_cfg.backLight / 100.0);
            // 设置屏幕日间亮度
            screen.setBackLight(app_controller->sys_cfg.backLight / 100.0);
        }
    }
}
// 任务1：专门处理亮度检查
void TaskBrightnessCheck(void *parameter)
{
    const TickType_t xDelay = 5000 / portTICK_PERIOD_MS;
    
    for (;;)
    {
        checkAndUpdateBrightness();
        vTaskDelay(xDelay);
    }
}

// 任务2：专门处理时间同步
void TaskTimeSync(void *parameter)
{
    const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;  // 每分钟检查一次是否需要同步
    
    unsigned long lastSuccessfulSync = 0;
    const unsigned long SYNC_INTERVAL = 12 * 3600 * 1000;
    bool syncSucceeded = false;
    
    // 首次同步
    if (initTime()) {
        syncSucceeded = true;
        lastSuccessfulSync = millis();
    }
    
    for (;;)
    {
        unsigned long currentMillis = millis();
        
        // 检查是否需要同步
        if (!syncSucceeded || (currentMillis - lastSuccessfulSync > SYNC_INTERVAL)) 
        {
            Serial.print("执行时间同步...");
            if (initTime()) {
                syncSucceeded = true;
                lastSuccessfulSync = currentMillis;
                Serial.println("成功");
            } else {
                syncSucceeded = false;
                Serial.println("失败");
            }
        }
        
        vTaskDelay(xDelay);
    }
}


void setup()
{
    Serial.begin(115200);

    Serial.println(F("\nAIO (All in one) version " AIO_VERSION "\n"));
    Serial.flush();
    // MAC ID可用作芯片唯一标识
    Serial.print(F("ChipID(EfuseMac): "));
    Serial.println(ESP.getEfuseMac());
    // flash运行模式
    // Serial.print(F("FlashChipMode: "));
    // Serial.println(ESP.getFlashChipMode());
    // Serial.println(F("FlashChipMode value: FM_QIO = 0, FM_QOUT = 1, FM_DIO = 2, FM_DOUT = 3, FM_FAST_READ = 4, FM_SLOW_READ = 5, FM_UNKNOWN = 255"));

    app_controller = new AppController(); // APP控制器

    // 初始化wifi
    app_controller->send_to(CTRL_NAME, SERVER_APP_NAME,
                         APP_MESSAGE_WIFI_CONN, NULL, NULL);

    // 需要放在Setup里初始化
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

#ifdef PEAK
    pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT);
    pinMode(CONFIG_ENCODER_PUSH_PIN, INPUT_PULLUP);
    /*电源使能保持*/
    Serial.println("Power: Waiting...");
    pinMode(CONFIG_POWER_EN_PIN, OUTPUT);
    digitalWrite(CONFIG_POWER_EN_PIN, LOW);
    digitalWrite(CONFIG_POWER_EN_PIN, HIGH);
    Serial.println("Power: ON");
    log_e("Power: ON");
#endif

    // config_read(NULL, &g_cfg);   // 旧的配置文件读取方式
    app_controller->read_config(&app_controller->sys_cfg);
    app_controller->read_config(&app_controller->mpu_cfg);
    app_controller->read_config(&app_controller->rgb_cfg);

    /*** Init screen ***/
    screen.init(app_controller->sys_cfg.rotation,
                app_controller->sys_cfg.backLight);

    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.05).setRGB(0, 64, 64);

    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();

    lv_fs_fatfs_init();

    // Update display in parallel thread.
    // BaseType_t taskLvglReturned = xTaskCreate(
    //     TaskLvglUpdate,
    //     "LvglThread",
    //     8 * 1024,
    //     nullptr,
    //     TASK_LVGL_PRIORITY,
    //     &handleTaskLvgl);
    // if (taskLvglReturned != pdPASS)
    // {
    //     Serial.println("taskLvglReturned != pdPASS");
    // }
    // else
    // {
    //     Serial.println("taskLvglReturned == pdPASS");
    // }

#if LV_USE_LOG
    lv_log_register_print_cb(my_print);
#endif /*LV_USE_LOG*/

    app_controller->init();

    // 将APP"安装"到controller里
#if APP_EXAMPLE_USE
    app_controller->app_install(&example_app);
#endif
#if APP_MYEXAMPLE_USE
    app_controller->app_install(&myexample_app);
#endif

#if APP_WEATHER_USE
    app_controller->app_install(&weather_app);
#endif

#if APP_WEATHER_OLD_USE
    app_controller->app_install(&weather_old_app);
#endif
#if APP_TOMATO_USE
    app_controller->app_install(&tomato_app);
#endif
#if APP_PICTURE_USE
    app_controller->app_install(&picture_app);
#endif
#if APP_MEDIA_PLAYER_USE
    app_controller->app_install(&media_app);
#endif
#if APP_SCREEN_SHARE_USE
    app_controller->app_install(&screen_share_app);
#endif
#if APP_FILE_MANAGER_USE
    app_controller->app_install(&file_manager_app);
#endif

    app_controller->app_install(&server_app);

#if APP_IDEA_ANIM_USE
    app_controller->app_install(&idea_app);
#endif
#if APP_BILIBILI_FANS_USE
    app_controller->app_install(&bilibili_app);
#endif
#if APP_SETTING_USE
    app_controller->app_install(&settings_app);
#endif
#if APP_GAME_2048_USE
    app_controller->app_install(&game_2048_app);
#endif
#if APP_GAME_SNAKE_USE
    app_controller->app_install(&game_snake_app);
#endif
#if APP_ANNIVERSARY_USE
    app_controller->app_install(&anniversary_app);
#endif
#if APP_HEARTBEAT_USE
    app_controller->app_install(&heartbeat_app, APP_TYPE_BACKGROUND);
#endif
#if APP_STOCK_MARKET_USE
    app_controller->app_install(&stockmarket_app);
#endif
#if APP_PC_RESOURCE_USE
    app_controller->app_install(&pc_resource_app);
#endif
#if APP_LHLXW_USE
    app_controller->app_install(&LHLXW_app);
#endif
    // 自启动APP
    app_controller->app_auto_start();

    // 优先显示屏幕 加快视觉上的开机时间
    app_controller->main_process(&mpu.action_info);

    /*** Init IMU as input device ***/
    // lv_port_indev_init();

    mpu.init(app_controller->sys_cfg.mpu_order,
             app_controller->sys_cfg.auto_calibration_mpu,
             &app_controller->mpu_cfg); // 初始化比较耗时

    /*** 以此作为MPU6050初始化完成的标志 ***/
    RgbConfig *rgb_cfg = &app_controller->rgb_cfg;
    // 初始化RGB灯 HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            rgb_cfg->min_value_0, rgb_cfg->min_value_1, rgb_cfg->min_value_2,
                            rgb_cfg->max_value_0, rgb_cfg->max_value_1, rgb_cfg->max_value_2,
                            rgb_cfg->step_0, rgb_cfg->step_1, rgb_cfg->step_2,
                            rgb_cfg->min_brightness, rgb_cfg->max_brightness,
                            rgb_cfg->brightness_step, rgb_cfg->time, 
                            rgb_cfg->brightness_night_mode_specified,
                            rgb_cfg->brightness_night_mode_start,
                            rgb_cfg->brightness_night_mode_end};
    // 运行RGB任务
    set_rgb_and_run(&rgb_setting, RUN_MODE_TASK);

    // 先初始化一次动作数据 防空指针
    act_info = mpu.getAction();
    // 定义一个mpu6050的动作检测定时器
    xTimerAction = xTimerCreate("Action Check",
                                200 / portTICK_PERIOD_MS,
                                pdTRUE, (void *)0, actionCheckHandle);
    xTimerStart(xTimerAction, 0);

    
    // 创建亮度检查任务
    xTaskCreate(
        TaskBrightnessCheck,
        "BrightnessCheck",
        2048,
        NULL,
        1,
        NULL
    );
    
    // 创建时间同步任务
    xTaskCreate(
        TaskTimeSync,
        "TimeSync",
        4096,  // 可能需要更多栈空间用于网络操作
        NULL,
        2,     // 优先级可以比亮度检查高一点
        NULL
    );
    
}




void loop()
{
    screen.routine();
    // 定期检查亮度模式

#ifdef PEAK
    // 适配稚晖君的PEAK
    if (!mpu.Encoder_GetIsPush())
    {
        Serial.println("mpu.Encoder_GetIsPush()1");
        delay(1000);
        if (!mpu.Encoder_GetIsPush())
        {
            Serial.println("mpu.Encoder_GetIsPush()2");
            // 适配Peak的关机功能
            digitalWrite(CONFIG_POWER_EN_PIN, LOW);
        }
    }
#endif
    if (isCheckAction)
    {
        isCheckAction = false;
        act_info = mpu.getAction();
    }
    app_controller->main_process(act_info); // 运行当前进程
    // Serial.println(ambLight.getLux() / 50.0);
    // rgb.setBrightness(ambLight.getLux() / 500.0);
}