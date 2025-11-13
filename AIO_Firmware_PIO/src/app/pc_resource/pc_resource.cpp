#include "pc_resource.h"
#include "pc_resource_gui.h"
#include "ESP32Time.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include "ArduinoJson.h"

#define PC_RESOURCE_APP_NAME "PC Resource"

// Data parsing table - header strings
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
// Data parsing table - units
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

// Persistent configuration for sensor component
#define PC_RESOURCE_CONFIG_PATH "/pc_resource.cfg"
struct PCS_Config
{
    String pc_ipaddr;                   // PC's LAN IP address
    unsigned long sensorUpdataInterval; // Sensor data update interval (ms)
};

struct PCResourceAppRunData
{
    WiFiClient *client;            // WiFi client
    const char *host;              // Host IP address
    unsigned long preSensorMillis; // Milliseconds of last sensor data update
    unsigned long preTimeMillis;   // Update time counter
    PC_Resource rs_data;           // Remote sensor data
};

enum rs_event_Id
{
    UPDATE_RS_DATA,
};

// APP runtime data
static PCResourceAppRunData *run_data = NULL;

// Configuration data
static PCS_Config cfg_data;

static void write_config(PCS_Config *cfg)
{
    char tmp[16];
    // Save configuration data to file (persistence)
    String w_data;
    w_data = w_data + cfg->pc_ipaddr + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->sensorUpdataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(PC_RESOURCE_CONFIG_PATH, w_data.c_str());
}

static void read_config(PCS_Config *cfg)
{
    // If persistent configuration file is needed, call this function to store data in flash
    // Configuration file name should start with APP name and end with ".cfg" to avoid conflicts between multiple APPs
    char info[64] = {0};
    uint16_t size = g_flashCfg.readFile(PC_RESOURCE_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // Default values
        cfg->pc_ipaddr = "0.0.0.0";
        cfg->sensorUpdataInterval = 1000; // Sensor data update interval 1000ms (1s)
        write_config(cfg);
    }
    else
    {
        // Parse data
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->pc_ipaddr = param[0];
        cfg->sensorUpdataInterval = atol(param[1]);
    }
}

/**
 * @brief Remote sensor data parsing
 */
static void pc_resource_data_del(String line)
{
    int16_t dataStart = 0;
    int16_t dataEnd = 0;
    String dataStr;
    int data[11];

    // Parse data
    for (int i = 0; i < 11; i++)
    {
        dataStart = line.indexOf(rs_data_header[i]) + strlen(rs_data_header[i]); // Find header string
        dataEnd = line.indexOf(rs_data_unit[i], dataStart);                      // Find unit string
        dataStr = line.substring(dataStart, dataEnd);
        data[i] = dataStr.toFloat() * 10; // Get integer data scaled by 10
    }

    // Load data
    run_data->rs_data.cpu_usage = data[0] / 10; // CPU utilization
    run_data->rs_data.cpu_temp = data[1];       // CPU temperature (scaled by 10)
    run_data->rs_data.cpu_freq = data[2] / 10;  // CPU frequency
    run_data->rs_data.cpu_power = data[3];      // CPU power consumption (scaled by 10)

    run_data->rs_data.gpu_usage = data[4] / 10; // GPU utilization
    run_data->rs_data.gpu_temp = data[5];       // GPU temperature (scaled by 10)
    run_data->rs_data.gpu_power = data[6];      // GPU power consumption (scaled by 10)

    run_data->rs_data.ram_usage = data[7] / 10; // RAM usage rate
    run_data->rs_data.ram_use = data[8] / 10;   // RAM used amount

    run_data->rs_data.net_upload_speed = data[9];    // Network upload speed
    run_data->rs_data.net_download_speed = data[10]; // Network download speed
}

/**
 * @brief Get remote sensor data
 */
static void get_pc_resource_data(void)
{
    if (WL_CONNECTED != WiFi.status()) // WiFi not connected
        return;

    Serial.print("connect host: " + cfg_data.pc_ipaddr);
    // Try to connect to host via IP address
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

    // Parse data
    pc_resource_data_del(line);
}

/**
 * @brief App initialization
 */
static int pc_resource_init(AppController *sys)
{
    tft->setSwapBytes(true);
    display_pc_resource_gui_init();
    // Get configuration
    read_config(&cfg_data);

    // Initialize runtime parameters
    run_data = (PCResourceAppRunData *)calloc(1, sizeof(PCResourceAppRunData));
    run_data->preSensorMillis = 0;
    run_data->preTimeMillis = 0;
    memset(&run_data->rs_data, 0, sizeof(PC_Resource));
    run_data->host = cfg_data.pc_ipaddr.c_str();
    run_data->client = new WiFiClient();

    return 0;
}

/**
 * @brief App process
 */
static void pc_resource_process(AppController *sys, const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    // Refresh display
    if (doDelayMillisTime(cfg_data.sensorUpdataInterval, &run_data->preTimeMillis, false))
    {
        // Send event to update data display
        sys->send_to(PC_RESOURCE_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, (void *)UPDATE_RS_DATA, NULL);
    }

    delay(30);
}

/**
 * @brief App background task
 */
static void pc_resource_background_task(AppController *sys, const ImuAction *act_info)
{
    // null
}

/**
 * @brief App exit callback function
 */
static int pc_resource_exit_callback(void *param)
{
    pc_resource_gui_release();

    if (1 == run_data->client->connected()) // Server connected
        run_data->client->stop();

    // Release runtime data
    if (NULL != run_data)
    {
        delete run_data->client;
        free(run_data);
        run_data = NULL;
    }

    return 0;
}

/**
 * @brief App message handler
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
