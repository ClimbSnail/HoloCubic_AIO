/***************************************************
  HoloCubic多功能固件源码

  聚合多种APP，内置天气、时钟、相册、特效动画、视频播放、视频投影、
  浏览器文件修改。（各APP具体使用参考说明书）

  Github repositories：https://github.com/ClimbSnail/HoloCubic_AIO

  Last review/edit by ClimbSnail: 2021/08/21
 ****************************************************/

#include "driver/lv_port_indev.h"
#include "driver/lv_port_fs.h"

#include "common.h"
#include "sys/app_controller.h"
#include "app/app_conf.h"



#include <SPIFFS.h>
#include <esp32-hal.h>
#include <esp32-hal-timer.h>

#include "MyMQTT.h"

static bool isCheckAction = false;

/*** Component objects **7*/
ImuAction *act_info;           // 存放mpu6050返回的数据
AppController *app_controller; // APP控制器

TaskHandle_t handleTaskLvgl;

MyMQTT *myMqtt;

void response(char *topic, char *responseName, uint8_t result);
void set_mood(int mood);
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void update_MQTT(void *pVoid);

int timout_cnt = 0;

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
#if APP_WEATHER_USE
    app_controller->app_install(&weather_app);
#endif
#if APP_TOMATO_USE
    app_controller->app_install(&tomato_app);
#endif
#if APP_WEATHER_OLD_USE
    app_controller->app_install(&weather_old_app);
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

#if APP_MOOD_USE
    app_controller->app_install(&mood_app);
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
                            rgb_cfg->brightness_step, rgb_cfg->time};
    // 运行RGB任务
    set_rgb_and_run(&rgb_setting, RUN_MODE_TASK);

    // 先初始化一次动作数据 防空指针
    act_info = mpu.getAction();
    // 定义一个mpu6050的动作检测定时器
    xTimerAction = xTimerCreate("Action Check",
                                200 / portTICK_PERIOD_MS,
                                pdTRUE, (void *)0, actionCheckHandle);
    xTimerStart(xTimerAction, 0);



//    g_network.open_ap(AP_SSID);




    //开辟多线程来进行更新mqtt
    xTaskCreate(update_MQTT,"mqtt",10000,NULL,1,NULL);

}


void update_MQTT(void *pVoid){
    myMqtt = new MyMQTT(app_controller->sys_cfg.ssid_0.c_str(),app_controller->sys_cfg.password_0.c_str(),mqtt_callback);


    while (true){
        myMqtt->loop();
    }
}

void loop()
{
    screen.routine();

#ifdef PEAK
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




void mqtt_callback(char *topic, byte *payload, unsigned int length){


    //从topic中获取request_id，以=分割
    char *request_id = strtok(topic,"=");
    request_id = strtok(nullptr,"=");


    payload[length] = '\0';

    String strPayload((const char *)payload);


    //使用ArduinoJson解析数据
    DynamicJsonDocument doc(strPayload.length() * 2);
    DeserializationError error = deserializeJson(doc, strPayload);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }


    //获取命令名称
    const char* commandName = doc["command_name"].as<const char *>();

    if (strcmp(commandName,"send_mood") == 0) {
        int nowMood = doc["paras"]["now_mood"].as<int>();
        Serial.println(nowMood);
        set_mood(nowMood);
    }


    //获取回复命令的接口
    char *response_topic = new char[255];
    strcpy(response_topic,topic_Commands_Response);
    strcat(response_topic,request_id);

    //回复命令
    response(response_topic,"change",SUCCESS);

}

/**
 * set_mood函数主要功能为在mqtt回调函数中调用，
 * 用来唤醒mood_app
 * @param mood
 */
void set_mood(int mood) {

    Serial.println(mood);

    app_controller->sys_cfg.current_mood = mood;

#include "app/mood/mood.h"


    app_controller->app_start(MOOD_APP_NAME);

}

void response(char *topic, char *responseName, uint8_t result) {
    /*发送命令响应部分*/
    /*构建JSON内容*/
    DynamicJsonDocument doc(1024);

    if(result == SUCCESS){
        doc["result_code"] = 0;

        //构建子JSON
        JsonObject paras = doc.createNestedObject("paras");

        paras["status"] = 200;
        paras["msg"] = "success";
    }
    else if(result == FAIL){
        doc["result_code"] = 1;
        JsonObject paras = doc.createNestedObject("paras");
        paras["status"] = 400;
        paras["msg"] = "fail";
    }

    doc["response_name"] = responseName;



    String strResponse;
    serializeJson(doc,strResponse);


    if (myMqtt->client.publish(topic, strResponse.c_str())) {
        Serial.println("Success sending response command message");
    } else {
        Serial.println("Error sending response command message");
    }



}

