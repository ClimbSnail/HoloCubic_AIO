//名称：2.4-TFT彩屏投屏下位机程序—DMA
//作者：大超
//B站：super大大怪i
//时间：2021/7/30
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <TJpg_Decoder.h>
#include <pgmspace.h>

uint16_t PROGMEM dmaBuffer1[32 * 32]; // Toggle buffer for 32*32 MCU block, 1024bytes
uint16_t PROGMEM dmaBuffer2[32 * 32]; // Toggle buffer for 32*32 MCU block, 1024bytes
uint16_t *dmaBufferPtr = dmaBuffer1;
bool dmaBufferSel = 0;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft);
char *ssid = "superstar";              //填写你的wifi名字
char *password = "123456789";          //填写你的wifi密码
char *service_ip = "192.168.31.120";   //上位机IP
int httpPort = 8080;                   //设置上位机端口
WiFiClient client;                     //初始化一个客户端对象
uint8_t buff[7000] PROGMEM = {0};      //每一帧的临时缓存
uint8_t img_buff[50000] PROGMEM = {0}; //用于存储tcp传过来的图片
uint16_t size_count = 0;               //计算一帧的字节大小

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= tft.height())
        return 0;

    // Double buffering is used, the bitmap is copied to the buffer by pushImageDMA() the
    // bitmap can then be updated by the jpeg decoder while DMA is in progress
    if (dmaBufferSel)
        dmaBufferPtr = dmaBuffer2;
    else
        dmaBufferPtr = dmaBuffer1;
    dmaBufferSel = !dmaBufferSel; // Toggle buffer selection
    //  pushImageDMA() will clip the image block at screen boundaries before initiating DMA
    tft.pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr); // Initiate DMA - blocking only if last DMA is not complete
    return 1;
}

byte loadNum = 6;
void loading(byte delayTime)
{ //启动动画
    clk.setColorDepth(8);

    clk.createSprite(200, 50);
    clk.fillSprite(0x0000);

    clk.drawRoundRect(0, 0, 200, 16, 8, 0xFFFF);
    clk.fillRoundRect(3, 3, loadNum, 10, 5, 0xFFFF);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_GREEN, 0x0000);
    clk.drawString("Connecting to WiFi", 100, 40, 2);
    clk.pushSprite(20, 67); //20 110
    clk.deleteSprite();
    loadNum += 1;
    if (loadNum >= 195)
    {
        loadNum = 195;
    }
    delay(delayTime);
}
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    tft.begin();
    tft.initDMA();
    tft.setRotation(1);        //横屏
    tft.fillScreen(TFT_BLACK); //黑色
    tft.setTextColor(TFT_BLACK, TFT_WHITE);

    WiFi.begin(ssid, password); //连接wifi
    delay(1000);                //等待1秒
    while (WiFi.status() != WL_CONNECTED)
    {
        for (byte n = 0; n < 10; n++)
        { //每500毫秒检测一次状态
            loading(50);
        }
    }
    while (loadNum < 195)
    { //让动画走完
        loading(3);
    }
    if (WiFi.status() == WL_CONNECTED) //判断如果wifi连接成功
    {
        client.connect(service_ip, httpPort); //连接到上位机
        //client.setNoDelay(false);//关闭Nagle算法
        Serial.println("wifi is connected!");
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        IPAddress ip = WiFi.localIP();
        Serial.print("IP Address: ");
        Serial.println(ip);
    }
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output); //解码成功回调函数
    client.write("ok");              //向上位机发送下一帧发送指令
}
uint16_t read_count = 0;       //读取buff的长度
uint8_t pack_size[2];          //用来装包大小字节
uint16_t frame_size;           //当前帧大小
float start_time, end_time;    //帧处理开始和结束时间
float receive_time, deal_time; //帧接收和解码时间

void loop()
{
    // put your main code here, to run repeatedly:
    //沾包问题 recv阻塞，长时间收不到数据就会断开
    //断开连接原因，读取buff太快，上位机发送太快造成buff溢出，清空缓冲区会断开（FLUSH）
    client.write("no"); //向上位机发送当前帧未写入完指令
    while (client.available())
    {
        while (client.available())
        { //检测缓冲区是否有数据
            if (read_count == 0)
            {
                start_time = millis();
                client.read(pack_size, 2); //读取帧大小
                frame_size = pack_size[0] + (pack_size[1] << 8);
            }
            read_count = client.read(buff, 7000);                                                                         //向缓冲区读取数据
            memcpy(&img_buff[size_count], buff, read_count);                                                              //将读取的buff字节地址复制给img_buff数组
            size_count = size_count + read_count;                                                                         //计数当前帧字节位置
                                                                                                                          //           Serial.println(size_count);
            if (img_buff[frame_size - 3] == 0xaa && img_buff[frame_size - 2] == 0xbb && img_buff[frame_size - 1] == 0xcc) //判断末尾数据是否当前帧校验位
            {
                receive_time = millis() - start_time;
                deal_time = millis();
                img_buff[frame_size - 3] = 0;
                img_buff[frame_size - 2] = 0;
                img_buff[frame_size - 1] = 0;                      //清除标志位
                tft.startWrite();                                  //必须先使用startWrite，以便TFT芯片选择保持低的DMA和SPI通道设置保持配置
                TJpgDec.drawJpg(0, 0, img_buff, sizeof(img_buff)); //在左上角的0,0处绘制图像——在这个草图中，DMA请求在回调tft_output()中处理
                tft.endWrite();                                    //必须使用endWrite来释放TFT芯片选择和释放SPI通道吗
                                                                   //            memset(&img_buff,0,sizeof(img_buff));//清空buff
                size_count = 0;                                    //下一帧
                read_count = 0;
                client.write("ok");  //向上位机发送下一帧发送指令
                end_time = millis(); //计算mcu刷新一张图片的时间，从而算出1s能刷新多少张图，即得出最大刷新率
                Serial.printf("帧大小：%d ", frame_size);
                Serial.print("MCU处理速度：");
                Serial.print(1000 / (end_time - start_time), 2);
                Serial.print("Fps");
                Serial.printf("帧接收耗时:%.2fms,帧解码显示耗时:%.2fms\n", receive_time, (millis() - deal_time));
                break;
            }
        }
    }
}
