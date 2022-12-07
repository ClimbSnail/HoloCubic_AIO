#include "pc_resource.h"
#include "pc_resource_gui.h"
#include "ESP32Time.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include "ArduinoJson.h"

#define PC_RESOURCE_APP_NAME "PC Resource"

// 数据解析表 - 前导字符串
static const char *rs_data_header[] = {
    "CPU usage",
    "CPU temp",
    "CPU freq",
    "CPU power",
    "GPU usage",
    "GPU temp",
    "GPU power",
    "RAM usage",
    "RAM use",
    "NET upload speed",
    "NET download speed",
};
// 数据解析表 - 单位
static const char *rs_data_unit[] = {
    "%",
    "C",
    "MHz",
    "W",
    "%",
    "C",
    "W",
    "%",
    "MB",
    "KB/s",
    "KB/s",
};

// 传感器组件的持久化配置
#define PC_RESOURCE_CONFIG_PATH "/pc_resource.cfg"
struct PCS_Config
{
    String pc_ipaddr;                   // 电脑的内网IP地址
    unsigned long sensorUpdataInterval; // 传感器数据更新的时间间隔(ms)
};

struct PCResourceAppRunData
{
    WiFiClient *client;            // wifi客户端
    const char *host;              // 主机IP地址
    unsigned long preSensorMillis; // 上一回更新传感器数据时的毫秒数
    unsigned long preTimeMillis;   // 更新时间计数器
    PC_Resource rs_data;           // 遥感器数据
};

enum rs_event_Id
{
    UPDATE_RS_DATA,
};

// APP运行数据
static PCResourceAppRunData *run_data = NULL;

// 配置信息
static PCS_Config cfg_data;

static void write_config(PCS_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->pc_ipaddr + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->sensorUpdataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(PC_RESOURCE_CONFIG_PATH, w_data.c_str());
}

static void read_config(PCS_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[64] = {0};
    uint16_t size = g_flashCfg.readFile(PC_RESOURCE_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->pc_ipaddr = "0.0.0.0";
        cfg->sensorUpdataInterval = 1000; // 传感器数据更新的时间间隔1000(1s)
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->pc_ipaddr = param[0];
        cfg->sensorUpdataInterval = atol(param[1]);
    }
}

/**
 * @brief 遥感器数据解析
 */
static void pc_resource_data_del(String line)
{
    int16_t dataStart = 0;
    int16_t dataEnd = 0;
    String dataStr;
    int data[11];

    // 解析数据
    for (int i = 0; i < 11; i++)
    {
        dataStart = line.indexOf(rs_data_header[i]) + strlen(rs_data_header[i]); // 寻找前导字符串
        dataEnd = line.indexOf(rs_data_unit[i], dataStart);                      // 寻找单位字符串
        dataStr = line.substring(dataStart, dataEnd);
        data[i] = dataStr.toFloat() * 10; // 得到扩大10倍的整型数据
    }

    // 装载数据
    run_data->rs_data.cpu_usage = data[0] / 10; // CPU利用率
    run_data->rs_data.cpu_temp = data[1];       // CPU温度(扩大10倍)
    run_data->rs_data.cpu_freq = data[2] / 10;  // CPU主频
    run_data->rs_data.cpu_power = data[3];      // CPU功耗(扩大10倍)

    run_data->rs_data.gpu_usage = data[4] / 10; // GPU利用率
    run_data->rs_data.gpu_temp = data[5];       // GPU温度(扩大10倍)
    run_data->rs_data.gpu_power = data[6];      // GPU功耗(扩大10倍)

    run_data->rs_data.ram_usage = data[7] / 10; // RAM使用率
    run_data->rs_data.ram_use = data[8] / 10;   // RAM使用量

    run_data->rs_data.net_upload_speed = data[9];    // net上行速率
    run_data->rs_data.net_download_speed = data[10]; // net下行速率
}

/**
 * @brief 获取遥感器数据
 */
static void get_pc_resource_data(void)
{
    if (WL_CONNECTED != WiFi.status()) // WIFI未连接
        return;

    Serial.print("connect host: " + cfg_data.pc_ipaddr);
    // 尝试通过IP地址连接主机
    if (!run_data->client->connect(run_data->host, 80, 200))
    {
        Serial.println("Connect host failed!");
        return;
    }
    else
        Serial.println("host Conected!");

    String getUrl = "/sse";
    run_data->client->print(String("GET ") + getUrl + " HTTP/1.1\r\n" +
                            "Content-Type=application/json;charset=utf-8\r\n" + "Host: " +
                            run_data->host + "\r\n" + "User-Agent=ESP32\r\n" + "Connection: close\r\n\r\n");

    Serial.println("Get send");
    delay(10);

    char endOfHeaders[] = "\n\n";
    bool ok = run_data->client->find(endOfHeaders);
    if (!ok)
    {
        Serial.println("No response or invalid response!");

        return;
    }
    Serial.println("Skip headers");

    String line = "";
    line += run_data->client->readStringUntil('\n');
    run_data->client->stop();

    Serial.println("Content:");
    Serial.println(line);

    // 解析数据
    pc_resource_data_del(line);
}

/**
 * @brief app初始化
 */
static int pc_resource_init(AppController *sys)
{
    tft->setSwapBytes(true);
    display_pc_resource_gui_init();
    // 获取配置信息
    read_config(&cfg_data);

    // 初始化运行时参数
    run_data = (PCResourceAppRunData *)calloc(1, sizeof(PCResourceAppRunData));
    run_data->preSensorMillis = 0;
    run_data->preTimeMillis = 0;
    memset(&run_data->rs_data, 0, sizeof(PC_Resource));
    run_data->host = cfg_data.pc_ipaddr.c_str();
    run_data->client = new WiFiClient();

    return 0;
}

/**
 * @brief app进程
 */
static void pc_resource_process(AppController *sys, const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    // 刷新显示
    if (doDelayMillisTime(cfg_data.sensorUpdataInterval, &run_data->preTimeMillis, false))
    {
        // 发送更新数据显示事件
        sys->send_to(PC_RESOURCE_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, (void *)UPDATE_RS_DATA, NULL);
    }

    delay(30);
}

/**
 * @brief app后台任务
 */
static void pc_resource_background_task(AppController *sys, const ImuAction *act_info)
{
    // null
}

/**
 * @brief app退出回调函数
 */
static int pc_resource_exit_callback(void *param)
{
    pc_resource_gui_release();

    if (1 == run_data->client->connected()) // 服务器已连接
        run_data->client->stop();

    // 释放运行数据
    if (NULL != run_data)
    {
        delete run_data->client;
        free(run_data);
        run_data = NULL;
    }

    return 0;
}

/**
 * @brief app消息处理
 */
static void pc_resource_message_handle(const char *from, const char *to,
                                       APP_MESSAGE_TYPE type, void *message,
                                       void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.println(F("----->pc_resource_event_notification"));
        int event_id = (int)message;
        switch (event_id)
        {
        case UPDATE_RS_DATA:
        {
            Serial.print(F("pc_resource update data.\n"));

            get_pc_resource_data();
            display_pc_resource(run_data->rs_data);
        };
        break;
        default:
            break;
        }
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "pc_ipaddr"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.pc_ipaddr.c_str());
        }
        else if (!strcmp(param_key, "sensorUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.sensorUpdataInterval);
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "pc_ipaddr"))
        {
            cfg_data.pc_ipaddr = param_val;
        }
        else if (!strcmp(param_key, "sensorUpdataInterval"))
        {
            cfg_data.sensorUpdataInterval = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ pc_resource_app = {PC_RESOURCE_APP_NAME, &app_pc_resource, "",
                           pc_resource_init, pc_resource_process, pc_resource_background_task,
                           pc_resource_exit_callback, pc_resource_message_handle};
