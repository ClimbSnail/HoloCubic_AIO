#include "screen_share.h"
#include "screen_share_gui.h"
#include "common.h"
#include <TJpg_Decoder.h>
#include "sys/app_contorller.h"

#define JPEG_BUFFER_SIZE 1     // 10000 // 储存一张jpeg的图像(240*240 10000大概够了，正常一帧差不多3000)
#define RECV_BUFFER_SIZE 50000 // 理论上是JPEG_BUFFER_SIZE的两倍就够了
#define DMA_BUFFER_SIZE 512    // (16*16*2)

#define HTTP_PORT 8081          //设置监听端口
WiFiServer screen_share_server; //服务端
WiFiClient client;              // 客户端

boolean tcp_start = 0;       // 标志是否开启web server服务，0为关闭 1为开启
static boolean req_sent = 0; // 标志是否发送wifi请求服务，0为关闭 1为开启

int32_t read_count = 0;        //读取buff的长度
uint8_t pack_size[2];          //用来装包大小字节
uint32_t frame_size;           //当前帧大小
float start_time, end_time;    //帧处理开始和结束时间
float receive_time, deal_time; //帧接收和解码时间

uint8_t *recvBuf;    // 显示的
uint8_t *jpegBuf;    // 用来给 jpeg 图片做缓冲，将此提交给jpeg解码器解码
int32_t bufSaveTail; // 指向 jpegBuf 中所保存的最后一个数据所在下标
uint8_t *displayBufWithDma[2];
bool dmaBufferSel = false;
static boolean tftSwapStatus;

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
    if (dmaBufferSel)
        dmaBufferPtr = (uint16_t *)displayBufWithDma[0];
    else
        dmaBufferPtr = (uint16_t *)displayBufWithDma[1];
    dmaBufferSel = !dmaBufferSel; // Toggle buffer selection
    //  pushImageDMA() will clip the image block at screen boundaries before initiating DMA
    tft->pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr); // Initiate DMA - blocking only if last DMA is not complete
                                                         // The DMA transfer of image block to the TFT is now in progress...

    // Return 1 to decode next block.
    return 1;
}

uint32_t readJpegFromFile(WiFiClient *client)
{
    int32_t read_size = 0;
    int32_t pos = 0;
    bool isFound = false;
    while (true)
    {
        // 查找帧
        for (; pos < bufSaveTail - 1; ++pos)
        {
            if (recvBuf[pos] == 0xFF && recvBuf[pos + 1] == 0xD9)
            {
                isFound = true;
                break;
            }
        }
        if (isFound)
        {
            // 找到一帧数据
            break;
        }
        read_size = client->read(&recvBuf[bufSaveTail], JPEG_BUFFER_SIZE);
        bufSaveTail += read_size;
    }
    memcpy(jpegBuf, recvBuf, pos + 2);
    // 把多余数据（本次没用上的数据保存下来）
    memcpy(recvBuf, &recvBuf[pos + 2], bufSaveTail - pos - 2);
    // 保存数据 下次循环再使用
    bufSaveTail = bufSaveTail - pos - 2;
    // Serial.println(pos + 2);
    return pos + 2;
}

void screen_share_init(void)
{
    // 设置CPU主频
    setCpuFrequencyMhz(240);

    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    set_rgb(&rgb_setting);

    screen_share_gui_init();
    recvBuf = NULL;
    jpegBuf = NULL;
    bufSaveTail = 0;
    recvBuf = (uint8_t *)malloc(RECV_BUFFER_SIZE);
    jpegBuf = (uint8_t *)malloc(JPEG_BUFFER_SIZE);
    displayBufWithDma[0] = (uint8_t *)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
    displayBufWithDma[1] = (uint8_t *)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
    tft->initDMA();

    // The decoder must be given the exact name of the rendering function above
    SketchCallback callback = (SketchCallback)&screen_share_tft_output; //强制转换func()的类型
    TJpgDec.setCallback(callback);
    // The jpeg image can be scaled down by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);

    tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true);
    // 因为其他app里是对tft直接设置的，所以此处尽量了不要使用TJpgDec的setSwapBytes
    // TJpgDec.setSwapBytes(true);
}

void stop_share_config()
{
    tcp_start = 0;
    req_sent = 0;
    screen_share_server.stop();
    screen_share_server.close();
}

void screen_share_process(AppController *sys,
                          const Imu_Action *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        stop_share_config();
        sys->app_exit();
        return;
    }

    if (0 == tcp_start && 0 == req_sent)
    {
        // 预显示
        display_screen_share(
            "Screen Share",
            WiFi.softAPIP().toString().c_str(),
            "8081",
            "Wait connect ....",
            LV_SCR_LOAD_ANIM_NONE);
        // 如果web服务没有开启 且 ap开启的请求没有发送 event_id这边没有作用（填0）
        // sys->req_event(&screen_share_app, APP_EVENT_WIFI_AP, 0);
        // 使用STA模式
        sys->req_event(&screen_share_app, APP_EVENT_WIFI_CONN, 0);
        req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == tcp_start)
    {
        // 发送wifi维持的心跳
        sys->req_event(&screen_share_app, APP_EVENT_WIFI_ALIVE, 0);
        if (client)
        {
            if (client.connected() || client.available()) // 如果客户端处于连接状态client.connected()
            {
                if (client.available())
                {
                    // uint8_t *recvBuf;   // 显示的
                    // uint8_t *jpegBuf;   //
                    client.write("no"); // 向上位机发送当前帧未写入完指令
                    // 检测缓冲区是否有数据
                    if (bufSaveTail == 0)
                    {
                        start_time = millis();
                        client.read(pack_size, 2); //读取帧大小
                        frame_size = pack_size[0] + (pack_size[1] << 8);
                    }
                    bufSaveTail = 0;
                    int time_cnt = 10000;
                    while (bufSaveTail < frame_size && --time_cnt)
                    {
                        if (bufSaveTail > RECV_BUFFER_SIZE - 7000)
                        {
                            client.read(recvBuf, 10000);
                            // 认定于接收失败 read_count清理
                            Serial.println("bufSaveTail > RECV_BUFFER_SIZE - 7000");
                            read_count = 0;
                            break;
                        }
                        read_count = client.read(&recvBuf[bufSaveTail], 7000); //向缓冲区读取数据
                        bufSaveTail += read_count;
                    }
                    bufSaveTail = 0;
                    client.write("ok"); // 向上位机发送下一帧发送指令
                    if (0 == read_count || time_cnt < 1)
                    {
                        return;
                    }
                    // 判断末尾数据是否当前帧校验位
                    if (recvBuf[frame_size - 3] == 0xaa && recvBuf[frame_size - 2] == 0xbb && recvBuf[frame_size - 1] == 0xcc)
                    {
                        receive_time = millis() - start_time;
                        deal_time = millis();
                        // tft->startWrite();                              // 必须先使用startWrite，以便TFT芯片选择保持低的DMA和SPI通道设置保持配置
                        TJpgDec.drawJpg(0, 0, recvBuf, frame_size - 3); // 在左上角的0,0处绘制图像——在这个草图中，DMA请求在回调tft_output()中处理
                        // tft->endWrite();                                // 必须使用endWrite来释放TFT芯片选择和释放SPI通道吗
                        end_time = millis(); // 计算mcu刷新一张图片的时间，从而算出1s能刷新多少张图，即得出最大刷新率
                        Serial.printf("帧大小：%d ", frame_size);
                        Serial.print("MCU处理速度：");
                        Serial.print(1000 / (end_time - start_time), 2);
                        Serial.print("Fps");
                        Serial.printf("帧接收耗时:%.2fms,帧解码显示耗时:%.2fms\n", receive_time, (millis() - deal_time));
                    }
                }
            }
            else if (!client.connected())
            {
                client.stop();
                Serial.println("Controller was disconnect!");
            }
        }
        else
        {
            // 建立客户
            client = screen_share_server.available();
            if (client)
            {
                Serial.println("Controller was connected!");
                client.write("ok"); // 向上位机发送下一帧发送指令
            }
        }
    }
}

void screen_exit_callback(void)
{
    screen_share_gui_del();
    if (NULL != recvBuf)
    {
        free(recvBuf);
        recvBuf = NULL;
    }
    if (NULL != jpegBuf)
    {
        free(jpegBuf);
        jpegBuf = NULL;
    }
    if (NULL != displayBufWithDma[0])
    {
        free(displayBufWithDma[0]);
        displayBufWithDma[0] = NULL;
    }
    if (NULL != displayBufWithDma[1])
    {
        free(displayBufWithDma[1]);
        displayBufWithDma[1] = NULL;
    }

    // 恢复此前的驱动参数
    tft->setSwapBytes(tftSwapStatus);

    // 恢复RGB灯  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            1, 32, 255,
                            255, 255, 255,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    set_rgb(&rgb_setting);
}

void screen_event_notification(APP_EVENT event, int event_id)
{
    switch (event)
    {
    case APP_EVENT_WIFI_CONN:
    {
        Serial.print(F("APP_EVENT_WIFI_AP enable\n"));
        display_screen_share(
            "Screen Share",
            WiFi.localIP().toString().c_str(),
            "8081",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        tcp_start = 1;
        screen_share_server.begin(HTTP_PORT); //服务器启动监听端口号
        screen_share_server.setNoDelay(true);
    }
    break;
    case APP_EVENT_WIFI_AP:
    {
        Serial.print(F("APP_EVENT_WIFI_AP enable\n"));
        display_screen_share(
            "Screen Share",
            WiFi.softAPIP().toString().c_str(),
            "8081",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        tcp_start = 1;
        // screen_share_server.begin(HTTP_PORT); //服务器启动监听端口号
        // screen_share_server.setNoDelay(true);
    }
    break;
    case APP_EVENT_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    default:
        break;
    }
}

APP_OBJ screen_share_app = {"Screen share", &app_screen, "", screen_share_init,
                            screen_share_process, screen_exit_callback,
                            screen_event_notification};