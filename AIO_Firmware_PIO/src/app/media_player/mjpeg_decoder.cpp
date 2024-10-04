#include "decoder.h"
#ifndef MJPEG_APP_OLD
// 新的非常占用内存
#include "common.h"
// #include <TJpg_Decoder.h>
// #include "MjpegClass.h"
// static MjpegClass mjpeg;

// #include "JPEGDEC/JPEGDEC.h"
#include "JPEGDEC.h"

#define VIDEO_WIDTH SCREEN_WIDTH
#define VIDEO_HEIGHT SCREEN_HEIGHT
#define EACH_READ_SIZE 2500     // 每次获取的数据流大小
#define JPEG_BUFFER_SIZE 10000  // 储存一张jpeg的图像(240*240 10000大概够了，正常一帧差不多3000)
#define MOVIE_BUFFER_SIZE 20000 // 理论上是JPEG_BUFFER_SIZE的两倍就够了

#define DMA_BUFFER_SIZE 512 // (16*16*2)
static JPEGDEC jpeg;

#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises

bool MjpegPlayDecoder::m_isUseDMA = 0;

uint8_t *displayBufWithDma;
// bool IRAM_ATTR tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
// {
//     // 解一帧 240*280的数据将调用该函数270次
//     // 解一帧 240*320的数据将调用该函数300次

//     memcpy(displayBufWithDma, bitmap, w * h * sizeof(uint16_t));
//     tft->pushImage(x, y, w, h, displayBufWithDma);
//     return true;
// }

int tft_output(JPEGDRAW *pDraw)
// int IRAM_ATTR tft_output(JPEGDRAW *pDraw)
{
    // 解一帧 240*280的数据将调用该函数270次
    // 解一帧 240*320的数据将调用该函数300次
    memcpy(displayBufWithDma, pDraw->pPixels,
           pDraw->iWidth * pDraw->iHeight * sizeof(uint16_t));
    tft->pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, displayBufWithDma);
    return true;
}

uint32_t MjpegPlayDecoder::readJpegFromFile(File *file, uint8_t *buf, int32_t &bufSaveTail)
{
    int32_t read_size = 0;
    int32_t pos = 0;
    bool isFound = false;
    while (true)
    {
        // 查找帧
        for (; pos < bufSaveTail - 1; ++pos)
        {
            if (buf[pos] == 0xFF && buf[pos + 1] == 0xD9)
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

        if (bufSaveTail + EACH_READ_SIZE > MOVIE_BUFFER_SIZE)
        {
            // 防止本帧太大溢出，间接丢弃该帧
            bufSaveTail = 0;
            pos = 0;
        }
        read_size = file->read(&buf[bufSaveTail], EACH_READ_SIZE);
        if (ESP_FAIL == read_size)
        {
            // flash读取异常 未读取到数据
            // 避免死循环 跳出此函数
            return false;
        }
        bufSaveTail += read_size;
    }

    return pos + 2;
}

MjpegPlayDecoder::MjpegPlayDecoder(File *file, bool isUseDMA)
{
    m_pFile = file;
    m_isUseDMA = isUseDMA;
    m_displayBuf = NULL;
    m_bufSaveTail = 0;

    // TJpgDec.setJpgScale(1);
    // TJpgDec.setSwapBytes(true);
    // TJpgDec.setCallback((SketchCallback)tft_output);
    m_tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true);

    video_start();
}

MjpegPlayDecoder::~MjpegPlayDecoder(void)
{
    Serial.println(F("~MjpegPlayDecoder"));
    tft->setSwapBytes(m_tftSwapStatus);
    // 释放资源
    video_end();
}

bool MjpegPlayDecoder::video_start()
{
    // m_displayBuf = (uint8_t *)malloc(MOVIE_BUFFER_SIZE);
    m_displayBuf = (uint8_t *)heap_caps_malloc(MOVIE_BUFFER_SIZE, MALLOC_CAP_DMA);
    displayBufWithDma = (uint8_t *)heap_caps_malloc(1024, MALLOC_CAP_DMA);
    if (NULL == m_displayBuf)
    {
        Serial.println("ERROR NULL == m_displayBuf\n");
    }
    else
    {
        Serial.println("Succ NULL != m_displayBuf\n");
        Serial.printf("m_displayBuf addr = 0x%x\n", (unsigned int)m_displayBuf);
    }

    return true;
}

bool MjpegPlayDecoder::video_play_screen(void)
{
    uint32_t jpg_size = 0;

    long startTime = GET_SYS_MILLIS();
    long midTime;

    jpg_size = readJpegFromFile(m_pFile, m_displayBuf, m_bufSaveTail);

    if (jpeg.openRAM(m_displayBuf, jpg_size, tft_output))
    {
        // Serial.println("Successfully opened JPEG image");
        // Serial.println("Image size: %d x %d, orientation: %d, bpp: %d\n", jpeg.getWidth(),
        //        jpeg.getHeight(), jpeg.getOrientation(), jpeg.getBpp());
        // if (jpeg.hasThumb())
        //     Serial.println("Thumbnail present: %d x %d\n", jpeg.getThumbWidth(), jpeg.getThumbHeight());
        jpeg.setPixelType(RGB565_BIG_ENDIAN); // The SPI LCD wants the 16-bit pixels in big-endian order
        midTime = GET_SYS_MILLIS();
        // Draw the thumbnail image in the middle of the display (upper left corner = 120,100) at 1/4 scale
        if (jpeg.decode(0, 0, 0)) // JPEG_AUTO_ROTATE
        {
            // Serial.println("Successfully decoded image in %d us", GET_SYS_MILLIS() - midTime);
        }
        else
        {
            Serial.printf("Fail decoded image in %d us", jpeg.getLastError());
        }
        jpeg.close();
    }
    // Serial.println("ALl play %d us", GET_SYS_MILLIS() - startTime);

    // 把多余数据（本次没用上的数据保存下来）
    memcpy(m_displayBuf, &m_displayBuf[jpg_size], m_bufSaveTail - jpg_size);
    // 保存数据 下次循环再使用
    m_bufSaveTail = m_bufSaveTail - jpg_size;

    return true;
}

bool MjpegPlayDecoder::video_end(void)
{
    // 结束播放 释放资源
    m_pFile = NULL;

    if (NULL != m_displayBuf)
    {
        free(m_displayBuf);
        m_displayBuf = NULL;
    }

    return true;
}

#endif