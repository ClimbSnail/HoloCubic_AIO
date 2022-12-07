#include "stockmarket.h"
#include "stockmarket_gui.h"
#include "sys/app_controller.h"
#include "../../common.h"

// STOCKmarket的持久化配置
#define B_CONFIG_PATH "/stockmarket.cfg"
struct B_Config
{
    String stock_id;              // bilibili的uid
    unsigned long updataInterval; // 更新的时间间隔(s)
};

static void write_config(const B_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->stock_id + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->updataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(B_CONFIG_PATH, w_data.c_str());
}

static void read_config(B_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(B_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->stock_id = "sh601126";  // 股票代码
        cfg->updataInterval = 10000; // 更新的时间间隔10000(10s)
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->stock_id = param[0];
        cfg->updataInterval = atol(param[1]);
    }
    //    cfg->stock_id = "sh601126";  // 股票代码
}

struct StockmarketAppRunData
{
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
    StockMarket stockdata;
};

struct MyHttpResult
{
    int httpCode = 0;
    String httpResponse = "";
};

static B_Config cfg_data;
static StockmarketAppRunData *run_data = NULL;

static MyHttpResult http_request(String uid = "sh601126")
{
    String url = "http://hq.sinajs.cn/list=" + uid;
    MyHttpResult result;

    HTTPClient httpClient;
    httpClient.setTimeout(1000);
    bool status = httpClient.begin(url);
    if (status == false)
    {
        result.httpCode = -1;
        return result;
    }

    httpClient.addHeader("referer", "https://finance.sina.com.cn");
    int httpCode = httpClient.GET();
    String httpResponse = httpClient.getString();
    httpClient.end();
    result.httpCode = httpCode;
    result.httpResponse = httpResponse;
    return result;
}

static int stockmarket_init(AppController *sys)
{
    stockmarket_gui_init();
    // 获取配置信息
    read_config(&cfg_data);
    // 初始化运行时参数
    run_data = (StockmarketAppRunData *)malloc(sizeof(StockmarketAppRunData));
    run_data->stockdata.OpenQuo = 0;
    run_data->stockdata.CloseQuo = 0;
    run_data->stockdata.NowQuo = 0;
    run_data->stockdata.MaxQuo = 0;
    run_data->stockdata.MinQuo = 0;
    run_data->stockdata.ChgValue = 0;
    run_data->stockdata.ChgPercent = 0;
    run_data->stockdata.updownflag = 1;
    run_data->stockdata.name[0] = '\0';
    run_data->stockdata.code[0] = '\0';
    run_data->refresh_status = 0;
    run_data->stockdata.tradvolume = 0;
    run_data->stockdata.turnover = 0;
    run_data->refresh_time_millis = GET_SYS_MILLIS() - cfg_data.updataInterval;

    display_stockmarket(run_data->stockdata, LV_SCR_LOAD_ANIM_NONE);
    return 0;
}

static void stockmarket_process(AppController *sys,
                                const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;
    if (RETURN == act_info->active)
    {
        sys->send_to(STOCK_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_DISCONN, NULL, NULL);
        sys->app_exit(); // 退出APP
        return;
    }

    // 以下减少网络请求的压力
    if (doDelayMillisTime(cfg_data.updataInterval, &run_data->refresh_time_millis, false))
    {
        sys->send_to(STOCK_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
    }

    delay(300);
}

static void stockmarket_background_task(AppController *sys,
                                        const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int stockmarket_exit_callback(void *param)
{
    stockmarket_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void update_stock_data()
{
    MyHttpResult result = http_request(cfg_data.stock_id);
    if (-1 == result.httpCode)
    {
        Serial.println("[HTTP] Http request failed.");
        return;
    }
    if (result.httpCode > 0)
    {
        if (result.httpCode == HTTP_CODE_OK || result.httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = result.httpResponse;
            Serial.println("[HTTP] OK");
            Serial.println(payload);
            int startIndex_1 = payload.indexOf(',') + 1;
            int endIndex_1 = payload.indexOf(',', startIndex_1);
            int startIndex_2 = payload.indexOf(',', endIndex_1) + 1;
            int endIndex_2 = payload.indexOf(',', startIndex_2);
            int startIndex_3 = payload.indexOf(',', endIndex_2) + 1;
            int endIndex_3 = payload.indexOf(',', startIndex_3);
            int startIndex_4 = payload.indexOf(',', endIndex_3) + 1;
            int endIndex_4 = payload.indexOf(',', startIndex_4);
            int startIndex_5 = payload.indexOf(',', endIndex_4) + 1;
            int endIndex_5 = payload.indexOf(',', startIndex_5);
            String Stockname = payload.substring(payload.indexOf('"') + 1, payload.indexOf(',')); // 股票名称
            memset(run_data->stockdata.name, '\0', 9);
            for (int i = 0; i < 8; i++)
                run_data->stockdata.name[i] = Stockname.charAt(i);
            run_data->stockdata.name[8] = '\0';
            run_data->stockdata.OpenQuo = payload.substring(startIndex_1, endIndex_1).toFloat();  // 今日开盘价
            run_data->stockdata.CloseQuo = payload.substring(startIndex_2, endIndex_2).toFloat(); // 昨日收盘价
            run_data->stockdata.NowQuo = payload.substring(startIndex_3, endIndex_3).toFloat();   // 当前价
            run_data->stockdata.MaxQuo = payload.substring(startIndex_4, endIndex_4).toFloat();   // 今日最高价
            run_data->stockdata.MinQuo = payload.substring(startIndex_5, endIndex_5).toFloat();   // 今日最低价

            run_data->stockdata.ChgValue = run_data->stockdata.NowQuo - run_data->stockdata.CloseQuo;
            run_data->stockdata.ChgPercent = run_data->stockdata.ChgValue / run_data->stockdata.CloseQuo * 100;
            for (int i = 0; i < 8; i++)
                run_data->stockdata.code[i] = cfg_data.stock_id.charAt(i);

            if (run_data->stockdata.ChgValue >= 0)
            {
                run_data->stockdata.updownflag = 1;
            }
            else
            {
                run_data->stockdata.updownflag = 0;
            }
            int startIndex_6 = payload.indexOf(',', endIndex_5) + 1;
            int endIndex_6 = payload.indexOf(',', startIndex_6);
            int startIndex_7 = payload.indexOf(',', endIndex_6) + 1;
            int endIndex_7 = payload.indexOf(',', startIndex_7);
            int startIndex_8 = payload.indexOf(',', endIndex_7) + 1;
            int endIndex_8 = payload.indexOf(',', startIndex_8);
            int startIndex_9 = payload.indexOf(',', endIndex_8) + 1;
            int endIndex_9 = payload.indexOf(',', startIndex_9);
            run_data->stockdata.tradvolume = payload.substring(startIndex_8, endIndex_8).toFloat(); // 成交量
            run_data->stockdata.turnover = payload.substring(startIndex_9, endIndex_9).toFloat();   // 成交额
            // Serial.printf("chg= %.2f\r\n",run_data->stockdata.ChgValue);
            // Serial.printf("chgpercent= %.2f%%\r\n",run_data->stockdata.ChgPercent);
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
}

static void stockmarket_message_handle(const char *from, const char *to,
                                       APP_MESSAGE_TYPE type, void *message,
                                       void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.print(GET_SYS_MILLIS());
        Serial.println("[SYS] stockmarket_event_notification");
        update_stock_data();
        display_stockmarket(run_data->stockdata, LV_SCR_LOAD_ANIM_NONE);
    }
    break;
    case APP_MESSAGE_UPDATE_TIME:
    {
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "stock_id"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.stock_id.c_str());
        }
        else if (!strcmp(param_key, "updataInterval"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.updataInterval);
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "stock_id"))
        {
            cfg_data.stock_id = param_val;
        }
        else if (!strcmp(param_key, "updataInterval"))
        {
            cfg_data.updataInterval = atol(param_val);
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

APP_OBJ stockmarket_app = {STOCK_APP_NAME, &app_stockmarket, "", stockmarket_init,
                           stockmarket_process, stockmarket_background_task,
                           stockmarket_exit_callback, stockmarket_message_handle};
