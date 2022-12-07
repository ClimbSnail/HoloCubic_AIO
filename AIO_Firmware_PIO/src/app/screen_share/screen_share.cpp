#include "screen_share.h"
#include "screen_share_gui.h"
#include "common.h"
#include <TJpg_Decoder.h>
#include "sys/app_controller.h"

#define SCREEN_SHARE_APP_NAME "Screen share"

#define JPEG_BUFFER_SIZE 1       // 10000 // 储存一张jpeg的图像(240*240 10000大概够了，正常一帧差不多3000)
#define RECV_BUFFER_SIZE 50000   // 理论上是 JPEG_BUFFER_SIZE 的两倍就够了
#define DMA_BUFFER_SIZE 512      // (16*16*2)
#define SHARE_WIFI_ALIVE 20000UL // 维持wifi心跳的时间（20s）

#define HTTP_PORT 8081 // 设置监听端口
WiFiServer ss_server;  // 服务端 ss = screen_share
WiFiClient ss_client;  // 客户端 ss = screen_share

// 天气的持久化配置
#define SCREEN_SHARE_CONFIG_PATH "/screen_share.cfg"
struct SS_Config
{
    uint8_t powerFlag; // 功耗控制（0低发热 1性能优先）
};

static void write_config(SS_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->powerFlag);
    w_data += tmp;
    g_flashCfg.writeFile(SCREEN_SHARE_CONFIG_PATH, w_data.c_str());
}

static void read_config(SS_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(SCREEN_SHARE_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->powerFlag = 0; // 功耗控制（0低发热 1性能优先）
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[1] = {0};
        analyseParam(info, 1, param);
        cfg->powerFlag = atol(param[0]);
    }
}

struct ScreenShareAppRunData
{
    // 数据量也不大，同时为了数据结构清晰 这里不对其进行内存对齐了

    boolean tcp_start; // 标志是否开启web server服务，0为关闭 1为开启
    boolean req_sent;  // 标志是否发送wifi请求服务，0为关闭 1为开启

    uint8_t *recvBuf;              // 接收缓冲区
    uint8_t *mjpeg_start;          // 指向一帧mpjeg的图片的开头
    uint8_t *mjpeg_end;            // 指向一帧mpjeg的图片的结束
    uint8_t *last_find_pos;        // 上回查找到的位置
    int32_t bufSaveTail;           // 指向 recvBuf 中所保存的最后一个数据所在下标
    uint8_t *displayBufWithDma[2]; // 用于FDMA的两个缓冲区
    bool dmaBufferSel;             // dma的缓冲区切换标志
    boolean tftSwapStatus;

    unsigned long pre_wifi_alive_millis; // 上一次发送维持心跳的本地时间戳
};

static SS_Config cfg_data;
static ScreenShareAppRunData *run_data = NULL;

// This next function will be called during decoding of the jpeg file to render each
// 16x16 or 8x8 image tile (Minimum Coding Unit) to the tft->
bool screen_share_tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft->height())
        return 0;

    // STM32F767 processor takes 43ms just to decode (and not draw) jpeg (-Os compile option)
    // Total time to decode and also draw to TFT:
    // SPI 54MHz=71ms, with DMA 50ms, 71-43 = 28ms spent drawing, so DMA is complete before next MCU block is ready
    // Apparent performance benefit of DMA = 71/50 = 42%, 50 - 43 = 7ms lost elsewhere
    // SPI 27MHz=95ms, with DMA 52ms. 95-43 = 52ms spent drawing, so DMA is *just* complete before next MCU block is ready!
    // Apparent performance benefit of DMA = 95/52 = 83%, 52 - 43 = 9ms lost elsewhere

    // Double buffering is used, the bitmap is copied to the buffer by pushImageDMA() the
    // bitmap can then be updated by the jpeg decoder while DMA is in progress
    uint16_t *dmaBufferPtr;
    if (run_data->dmaBufferSel)
        dmaBufferPtr = (uint16_t *)run_data->displayBufWithDma[0];
    else
        dmaBufferPtr = (uint16_t *)run_data->displayBufWithDma[1];
    run_data->dmaBufferSel = !run_data->dmaBufferSel; // Toggle buffer selection
    //  pushImageDMA() will clip the image block at screen boundaries before initiating DMA
    tft->pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr); // Initiate DMA - blocking only if last DMA is not complete
                                                         // The DMA transfer of image block to the TFT is now in progress...

    // Return 1 to decode next block.
    return true;
}

static bool readJpegFromBuffer(uint8_t *const end)
{
    // 默认从 run_data->recvBuf 中读数据
    // end标记的当前最后一个数据的地址
    bool isFound = false;                     // 标志着是否找到一帧完整的mjpeg图像数据
    uint8_t *pfind = run_data->last_find_pos; // 开始查找的指针
    if (NULL == run_data->mjpeg_start)
    {
        // 没找到帧头的时候执行
        while (pfind < end)
        {
            if (*pfind == 0xFF && *(pfind + 1) == 0xD8)
            {
                run_data->mjpeg_start = pfind; // 帧头
                break;
            }
            ++pfind;
        }
        run_data->last_find_pos = pfind;
    }
    else if (NULL == run_data->mjpeg_end)
    {
        // 找帧尾
        while (pfind < end)
        {
            if (*pfind == 0xFF && *(pfind + 1) == 0xD9)
            {
                run_data->mjpeg_end = pfind + 1; // 帧头，标记的是最后一个0xD9
                isFound = true;
                break;
            }
            ++pfind;
        }
        run_data->last_find_pos = pfind;
    }
    return isFound;
}

static int screen_share_init(AppController *sys)
{
    // 获取配置信息
    read_config(&cfg_data);

    if (0 == cfg_data.powerFlag)
    {
        // 设置CPU主频
        setCpuFrequencyMhz(160);
    }
    else
    {
        setCpuFrequencyMhz(240);
    }

    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    set_rgb_and_run(&rgb_setting);

    screen_share_gui_init();
    // 初始化运行时参数
    run_data = (ScreenShareAppRunData *)calloc(1, sizeof(ScreenShareAppRunData));
    run_data->tcp_start = 0;
    run_data->req_sent = 0;
    run_data->recvBuf = (uint8_t *)malloc(RECV_BUFFER_SIZE);
    run_data->mjpeg_start = NULL;
    run_data->mjpeg_end = NULL;
    run_data->last_find_pos = run_data->recvBuf;
    run_data->bufSaveTail = 0;
    run_data->displayBufWithDma[0] = (uint8_t *)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
    run_data->displayBufWithDma[1] = (uint8_t *)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
    run_data->dmaBufferSel = false;
    run_data->pre_wifi_alive_millis = 0;

    tft->initDMA();

    // The decoder must be given the exact name of the rendering function above
    SketchCallback callback = (SketchCallback)&screen_share_tft_output; // 强制转换func()的类型
    TJpgDec.setCallback(callback);
    // The jpeg image can be scaled down by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);

    run_data->tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true);
    // 因为其他app里是对tft直接设置的，所以此处尽量了不要使用TJpgDec的setSwapBytes
    // TJpgDec.setSwapBytes(true);

    Serial.print(F("防止过热，目前为限制为中速档!\n"));
    return 0;
}

static void stop_share_config()
{
    run_data->tcp_start = 0;
    run_data->req_sent = 0;
    // 关闭服务端
    ss_server.stop();
    ss_server.close();
}

static void screen_share_process(AppController *sys,
                                 const ImuAction *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        sys->app_exit();
        return;
    }

    if (0 == run_data->tcp_start && 0 == run_data->req_sent)
    {
        // 预显示
        display_screen_share(
            "Screen Share",
            WiFi.softAPIP().toString().c_str(),
            "8081",
            "Wait connect ....",
            LV_SCR_LOAD_ANIM_NONE);
        // 如果web服务没有开启 且 ap开启的请求没有发送 message这边没有作用（填NULL）
        // sys->send_to(SCREEN_SHARE_APP_NAME, CTRL_NAME,
        //              APP_MESSAGE_WIFI_AP, NULL, NULL);
        // 使用STA模式
        sys->send_to(SCREEN_SHARE_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
        run_data->req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == run_data->tcp_start)
    {
        if (doDelayMillisTime(SHARE_WIFI_ALIVE, &run_data->pre_wifi_alive_millis, false))
        {
            // 发送wifi维持的心跳
            sys->send_to(SCREEN_SHARE_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_ALIVE, NULL, NULL);
        }

        if (ss_client.connected())
        {
            // 如果客户端处于连接状态client.connected()
            if (ss_client.available())
            {
                ss_client.write("no");                                                                 // 向上位机发送当前帧未写入完指令
                int32_t read_count = ss_client.read(&run_data->recvBuf[run_data->bufSaveTail], 10000); // 向缓冲区读取数据
                run_data->bufSaveTail += read_count;

                unsigned long deal_time = GET_SYS_MILLIS();
                bool get_mjpeg_ret = readJpegFromBuffer(run_data->recvBuf + run_data->bufSaveTail);

                if (true == get_mjpeg_ret)
                {
                    ss_client.write("ok"); // 向上位机发送下一帧发送指令
                    tft->startWrite();     // 必须先使用startWrite，以便TFT芯片选择保持低的DMA和SPI通道设置保持配置
                    uint32_t frame_size = run_data->mjpeg_end - run_data->mjpeg_start + 1;
                    // 在左上角的0,0处绘制图像——在这个草图中，DMA请求在回调tft_output()中处理
                    JRESULT jpg_ret = TJpgDec.drawJpg(0, 0, run_data->mjpeg_start, frame_size);
                    tft->endWrite(); // 必须使用endWrite来释放TFT芯片选择和释放SPI通道吗
                    // 剩余帧大小
                    uint32_t left_frame_size = &run_data->recvBuf[run_data->bufSaveTail] - run_data->mjpeg_end;
                    memcpy(run_data->recvBuf, run_data->mjpeg_end + 1, left_frame_size);
                    Serial.printf("帧大小：%d ", frame_size);
                    Serial.print("MCU处理速度：");
                    Serial.print(1000.0 / (GET_SYS_MILLIS() - deal_time), 2);
                    Serial.print("Fps\n");

                    run_data->last_find_pos = run_data->recvBuf;
                    run_data->bufSaveTail = 0;
                    // 数据清零
                    run_data->mjpeg_start = NULL;
                    run_data->mjpeg_end = NULL;
                }
                else if (run_data->bufSaveTail > RECV_BUFFER_SIZE)
                {
                    run_data->last_find_pos = run_data->recvBuf;
                    run_data->bufSaveTail = 0;
                    // 数据清零
                    run_data->mjpeg_start = NULL;
                    run_data->mjpeg_end = NULL;
                    ss_client.write("ok"); // 向上位机发送下一帧发送指令
                }
            }
        }
        else
        {
            // 建立客户
            ss_client = ss_server.available();
            if (ss_client.connected())
            {
                Serial.println(F("Controller was connected!"));
                ss_client.write("ok"); // 向上位机发送下一帧发送指令
            }

            // 预显示
            display_screen_share(
                "Screen Share",
                WiFi.localIP().toString().c_str(),
                "8081",
                "Wait connect ....",
                LV_SCR_LOAD_ANIM_NONE);

            unsigned long timeout = GET_SYS_MILLIS();
            while (ss_client.available() == 0)
            {
                if (GET_SYS_MILLIS() - timeout > 2000)
                {
                    Serial.print(F("Controller was disconnect!"));
                    Serial.println(F(" >>> Client Timeout !"));
                    ss_client.stop();
                    return;
                }
            }
        }
    }
}

static void screen_background_task(AppController *sys,
                                   const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int screen_exit_callback(void *param)
{
    stop_share_config();
    screen_share_gui_del();
    if (NULL != run_data->recvBuf)
    {
        free(run_data->recvBuf);
        run_data->recvBuf = NULL;
    }

    if (NULL != run_data->displayBufWithDma[0])
    {
        free(run_data->displayBufWithDma[0]);
        run_data->displayBufWithDma[0] = NULL;
    }
    if (NULL != run_data->displayBufWithDma[1])
    {
        free(run_data->displayBufWithDma[1]);
        run_data->displayBufWithDma[1] = NULL;
    }

    // 恢复此前的驱动参数
    tft->setSwapBytes(run_data->tftSwapStatus);

    // 恢复RGB灯  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            1, 32, 255,
                            255, 255, 255,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    set_rgb_and_run(&rgb_setting);

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void screen_message_handle(const char *from, const char *to,
                                  APP_MESSAGE_TYPE type, void *message,
                                  void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.print(F("APP_MESSAGE_WIFI_CONN enable\n"));
        display_screen_share(
            "Screen Share",
            WiFi.localIP().toString().c_str(),
            "8081",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
        ss_server.begin(HTTP_PORT); // 服务器启动监听端口号
        ss_server.setNoDelay(true);
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_screen_share(
            "Screen Share",
            WiFi.softAPIP().toString().c_str(),
            "8081",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
        // ss_server.begin(HTTP_PORT); //服务器启动监听端口号
        // ss_server.setNoDelay(true);
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "powerFlag"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.powerFlag);
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
        if (!strcmp(param_key, "powerFlag"))
        {
            cfg_data.powerFlag = atol(param_val);
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

APP_OBJ screen_share_app = {SCREEN_SHARE_APP_NAME, &app_screen, "",
                            screen_share_init, screen_share_process, screen_background_task,
                            screen_exit_callback, screen_message_handle};
