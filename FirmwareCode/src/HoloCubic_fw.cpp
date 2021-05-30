#include "display.h"
#include "imu.h"
#include "ambient.h"
#include "network.h"
#include "sd_card.h"
#include "rgb_led.h"
#include "lv_port_indev.h"
#include "lv_cubic_gui.h"
#include "lv_demo_encoder.h"
#include "lv_port_fatfs.h"
#include "ESP32Time.h"
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiMulti.h> // 当我们需要使用ESP8266开发板存储多个WiFi网络连接信息时，可以使用ESP8266WiFiMulti库来实现。

int image_pos = 0;  // 记录相册播放的历史
int clock_page = 0; // 时钟桌面的播放记录
unsigned long pic_perMillis = 0;
unsigned long preWifiClickMillis = 0;
unsigned long preWeatherMillis = 0;           // 上一回更新天气时的毫秒数
unsigned long preTimeMillis = 0;              // 上一回更新日期与时间时的毫秒数
unsigned long weatherUpdataInterval = 900000; // 天气更新的时间间隔
unsigned long timeUpdataInterval = 10000;     // 日期时钟更新的时间间隔
unsigned long wifiClickInterval = 15000;      // 日期时钟更新的时间间隔

//TimerHandle_t xTimer_ap;
struct Config
{
    String ssid;
    String password;
    String cityname;    // 显示的城市
    String language;    // 天气查询的地址编码
    String weather_key; // 知心天气api_key（秘钥）
};

Config g_cfg; // 全局配置文件
// 保存天气
Weather g_weather = {0, 0}; // 保存天气
/*** Component objects **7*/
Display screen;
IMU mpu;
Pixel rgb;
Ambient ambLight;
SdCard tf;
Imu_Active *act_info;            // 存放mpu6050返回的数据
int active_process_num = 0;
Network g_network;        // 网络连接
ESP32Time g_rtc;          // 用于时间解码
long int g_timestamp = 0; // 时间戳
const char *AP_SSID = "ESP32_Config"; //热点名称
const char *AP_PASS = "12345678";     //密码
#define ROOT_HTML "<!DOCTYPE html><html><head><title>WIFI Config by lwang</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><form method=\"GET\" action=\"connect\"><label class=\"input\"><span>WiFi SSID</span><input type=\"text\" name=\"ssid\"></label><label class=\"input\"><span>WiFi PASS</span><input type=\"text\"  name=\"pass\"></label><label class=\"input\"><span>City Name</span><input type=\"text\" name=\"cityname\"></label></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submie\"></form></body></html>"
WebServer server(80);

String unit = "c";
String time_api = "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp";

boolean doDelayMillisTime(long interval, unsigned long *previousMillis, boolean state);
void restCallback(TimerHandle_t xTimer);
void clock_app_process(Imu_Active *act_info);
void picture_app_process(Imu_Active *act_info);

//函数指针组
void (*p_processList[2])(Imu_Active *) = {clock_app_process, picture_app_process};
void controller(Imu_Active *act_info); // 对所有控制的通用处理
int processId = 0;

////####################################################################################################
//// 文件上传SD卡
////####################################################################################################
//void handleFileUpload(){//网络服务处理函数
//  upload_flag = true;//正在进行上传
//  if(server.uri() != "/") return;
//  HTTPUpload& upload = server.upload();
//  String filename;
//  char *file_sd;
//  if(upload.status == UPLOAD_FILE_START){//开启下载上传的文件
//    filename = upload.filename;
//    if(!filename.startsWith("/"))
//    {
//      filename = "/loge"+String(tft_num)+".jpg";//如果文件开头没有/则添加/ 并且对该图片添加计数尾缀
//      tft_num++;//文件数+1
//      EEPROM.write(20,tft_num);//将数据保存
//      EEPROM.commit();
//    }
//    Serial.print("handleFileUpload Name: "); Serial.println(filename);//打印文件名
//
//    SD.remove(filename);
//    fsUploadFile = SD.open(filename, "w");//将上传的文件保存
//    filename = String();
//  } else if(upload.status == UPLOAD_FILE_WRITE){
//    if(fsUploadFile)
//      fsUploadFile.write(upload.buf, upload.currentSize);//将上传文件写入SD卡
//  } else if(upload.status == UPLOAD_FILE_END){
//    if(fsUploadFile)
//      fsUploadFile.close();
//  }
//}
void UpdateWeather(void)
{
    g_weather = g_network.getWeather("https://api.seniverse.com/v3/weather/now.json?key=" + g_cfg.weather_key + "&location=" + g_cfg.cityname + "&language=" + g_cfg.language + "&unit=" + unit); //如果要改城市这里也需要修改
    Serial.println(g_weather.weather_code);
    Serial.println(g_cfg.cityname.c_str());
    char temperature[10] = {0};
    sprintf(temperature, "%d", g_weather.temperature);
    display_weather(g_cfg.cityname.c_str(), temperature, g_weather.weather_code);
}

void UpdateTime_RTC(void)
{
    g_timestamp = g_network.getTimestamp(time_api)+TIMEZERO_OFFSIZE; //nowapi时间API
    g_rtc.setTime(g_timestamp);
    String date = g_rtc.getDate(String("%Y-%m-%d"));
    String time = g_rtc.getTime(String("%H:%M"));

    Serial.println("-------------------------------------");
    Serial.println(date.c_str());
    Serial.println(time.c_str());
    Serial.println("-------------------------------------");
    display_time(date.c_str(), time.c_str());
}

void config_read(const char *file_path, Config *cfg)
{
    cfg->ssid = tf.readFileLine(file_path, 1);     // line-1 for WiFi ssid
    cfg->password = tf.readFileLine(file_path, 2); // line-2 for WiFi password
    cfg->cityname = tf.readFileLine(file_path, 3); // line-3 for cityname
    cfg->language = tf.readFileLine(file_path, 4); // line-2 for language
    cfg->weather_key = tf.readFileLine(file_path, 5); // line-3 for weather_key
    Serial.println(cfg->ssid);
    Serial.println(cfg->password);
    Serial.println(cfg->cityname);
    Serial.println(cfg->language);
    Serial.println(cfg->weather_key);
}

void config_save(const char *file_path, Config *cfg)
{
    Serial.println(cfg->ssid);
    Serial.println(cfg->password);
    Serial.println(cfg->cityname);
    Serial.println(cfg->language);
    Serial.println(cfg->weather_key);
    tf.deleteFile(file_path);
    tf.writeFile(file_path, cfg->ssid.c_str(), cfg->password.c_str(),
            cfg->cityname.c_str(), cfg->language.c_str(), cfg->weather_key.c_str());
}

void auto_config()
{
    //首页
    server.on("/", []()
              { server.send(200, "text/html", ROOT_HTML); });
    //连接
    server.on("/connect", []()
              {
                  server.send(200, "text/html", "<html><body><h1>successd,conning...</h1></body></html>");

                  WiFi.softAPdisconnect(true);

                  //获取输入的WIFI账户和密码
                  String ssid = server.arg("ssid");
                  String password = server.arg("pass");
                  String cityname = server.arg("cityname");
                  // 连接wifi
                  g_network.start_conn_wifi(ssid.c_str(), password.c_str());

                  xTimerStop(g_network.xTimer_ap, 0); //关掉超时定时器
                  // config_save("/wifi.txt", &g_cfg);
              });
    server.begin();
}

void wifi_auto_process()
{
    if (doDelayMillisTime(wifiClickInterval, &preWifiClickMillis, false) == true)
    {
        preWeatherMillis = millis() - weatherUpdataInterval;
        preTimeMillis = millis() - timeUpdataInterval;
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            if (AP_ENABLE == g_network.ap_status)
            {
                g_network.open_ap(AP_SSID, AP_PASS);
                auto_config();
            }
        }
    }
}

void controller(Imu_Active *act_info)
{
    // processId == 1 是相册功能
    if (UNKNOWN == act_info->active && 1 != processId)
        return;

    Serial.print("act_info->active: ");
    Serial.println(act_info->active);
    if (RETURN == act_info->active)
    {
        act_info->active = UNKNOWN;
        act_info->isValid = 0;
        processId = (processId + 1) % 2; // 此处的2与p_processList的长度一致
        //return;
    }

    (*(p_processList[processId]))(act_info);

    act_info->active = UNKNOWN;
    act_info->isValid = 0;
}

void clock_app_process(Imu_Active *act_info)
{

    Serial.println("-----------> clock_app_process");

    lv_scr_load_anim_t direction = LV_SCR_LOAD_ANIM_NONE;

    if (TURN_RIGHT == act_info->active)
    {
        direction = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        clock_page = (clock_page + 1) % 3;
    }
    else if (TURN_LEFT == act_info->active)
    {
        direction = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        clock_page = (clock_page - 1) % 3;
    }

    Serial.println("clock_page");
    Serial.println(clock_page);
    
    lv_scr_load_anim(scr[clock_page], direction, 500, 300, false);
    Serial.println("-----------> 6");

    if (0 == clock_page && millis() - preWeatherMillis >= weatherUpdataInterval)
    {
        // 更新天气
        preWeatherMillis = millis();
        UpdateWeather();
    }

    Serial.println("-----------> 3");
    if (1 == clock_page && millis() - preTimeMillis >= timeUpdataInterval)
    {
        // 更新时钟
        preTimeMillis = millis();
        UpdateTime_RTC();
    }
    Serial.println("-----------> 4");
    if (2 == clock_page)
    {
        // NULLk后期可以是具体数据
        display_hardware(NULL);
    }
}

void picture_app_process(Imu_Active *act_info)
{
    Serial.println("-----------> picture_app_process");
    lv_scr_load_anim_t direction = LV_SCR_LOAD_ANIM_NONE;

    // if (TURN_RIGHT == act_info->active)
    // {
    //     direction = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    // }
    // else if (TURN_LEFT == act_info->active)
    // {
    //     direction = LV_SCR_LOAD_ANIM_MOVE_LEFT;
    // }

    if (doDelayMillisTime(3000, &pic_perMillis, false) == true)
    {
        ++image_pos;
    }

    if (image_pos >= photo_file_num)
    {
        image_pos = 0;
    }

    display_photo(file_name_list[image_pos]);
    lv_scr_load_anim(scr[3], direction, 100, 0, false);
}

void setup()
{
    Serial.begin(115200);

    /*** Init screen ***/
    screen.init();
    screen.setBackLight(0.1);

    /*** Init IMU as input device ***/
    lv_port_indev_init();
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

    /*** Read WiFi info in SD-Card, then scan & connect WiFi ***/
    g_network.init(g_cfg.ssid, g_cfg.password);
    preWifiClickMillis = 0;

    display_init();
}

void loop()
{
    server.handleClient();
    // run this as often as possible ��
    UpdateWeather();
    act_info = mpu.update(200);
    act_info->active = GO_FORWORD;  // 让第一次能够执行刷新屏幕的判断
    while (true)
    {
        screen.routine();
        controller(act_info); // 运行当前进程
        delay(300);
        act_info = mpu.update(200);

        rgb.setBrightness(ambLight.getLux() / 500.0);
        wifi_auto_process(); // 任务调度
    }
}

boolean doDelayMillisTime(long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}
