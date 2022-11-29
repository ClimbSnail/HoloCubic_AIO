#include "docoder.h"
#include "common.h"
#include <TJpg_Decoder.h>
// #include "MjpegClass.h"
// static MjpegClass mjpeg;

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define EACH_READ_SIZE 2500     // 每次获取的数据流大小
#define JPEG_BUFFER_SIZE 10000  // 储存一张jpeg的图像(240*240 10000大概够了，正常一帧差不多3000)
#define MOVIE_BUFFER_SIZE 20000 // 理论上是JPEG_BUFFER_SIZE的两倍就够了

#define DMA_BUFFER_SIZE 512 // (16*16*2)

#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises

bool MjpegPlayDocoder::m_isUseDMA = 0;
uint8_t *MjpegPlayDocoder::m_displayBufWithDma[2];
bool MjpegPlayDocoder::m_dmaBufferSel = false;

// This next function will be called during decoding of the jpeg file to render each
// 16x16 or 8x8 image tile (Minimum Coding Unit) to the tft->
bool MjpegPlayDocoder::tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
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
    if (m_isUseDMA)
    {
        // Double buffering is used, the bitmap is copied to the buffer by pushImageDMA() the
        // bitmap can then be updated by the jpeg decoder while DMA is in progress
        uint16_t *dmaBufferPtr;
        if (MjpegPlayDocoder::m_dmaBufferSel)
            dmaBufferPtr = (uint16_t *)MjpegPlayDocoder::m_displayBufWithDma[0];
        else
            dmaBufferPtr = (uint16_t *)MjpegPlayDocoder::m_displayBufWithDma[1];
        MjpegPlayDocoder::m_dmaBufferSel = !MjpegPlayDocoder::m_dmaBufferSel; // Toggle buffer selection
        //  pushImageDMA() will clip the image block at screen boundaries before initiating DMA
        tft->pushImageDMA(x, y, w, h, bitmap, dmaBufferPtr); // Initiate DMA - blocking only if last DMA is not complete
                                                             // The DMA transfer of image block to the TFT is now in progress...
    }
    else
    {
        // Non-DMA blocking alternative
        tft->pushImage(x, y, w, h, bitmap); // Blocking, so only returns when image block is drawn
    }
    // Return 1 to decode next block.
    return 1;
}

uint32_t MjpegPlayDocoder::readJpegFromFile(File *file)
{
    int32_t read_size = 0;
    int32_t pos = 0;
    bool isFound = false;
    while (true)
    {
        // 查找帧
        for (; pos < m_bufSaveTail - 1; ++pos)
        {
            if (m_displayBuf[pos] == 0xFF && m_displayBuf[pos + 1] == 0xD9)
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
        if (m_bufSaveTail + EACH_READ_SIZE > MOVIE_BUFFER_SIZE)
        {
            // 防止本帧太大溢出，间接丢弃该帧
            m_bufSaveTail = 0;
            pos = 0;
        }
        read_size = file->read(&m_displayBuf[m_bufSaveTail], EACH_READ_SIZE);
        m_bufSaveTail += read_size;
    }

    if (pos + 2 < JPEG_BUFFER_SIZE)
    {
        // 只有帧大小小于 JPEG_BUFFER_SIZE 的时候才可以拷贝
        memcpy(m_jpegBuf, m_displayBuf, pos + 2);
    }
    // 把多余数据（本次没用上的数据保存下来）
    memcpy(m_displayBuf, &m_displayBuf[pos + 2], m_bufSaveTail - pos - 2);
    // 保存数据 下次循环再使用
    m_bufSaveTail = m_bufSaveTail - pos - 2;
    // Serial.println(pos + 2);
    return pos + 2;
}

MjpegPlayDocoder::MjpegPlayDocoder(File *file, bool isUseDMA)
{
    m_pFile = file;
    m_isUseDMA = isUseDMA;
    m_displayBuf = NULL;
    m_bufSaveTail = 0;
    m_jpegBuf = NULL;
    m_displayBufWithDma[0] = NULL;
    m_displayBufWithDma[1] = NULL;
    m_dmaBufferSel = 0;
    // The jpeg image can be scaled down by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);
    // The colour byte order can be swapped by the decoder
    // using TJpgDec.setSwapBytes(true); or by the TFT_eSPI library:
    m_tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true);
    // TJpgDec.setSwapBytes(true);
    // The decoder must be given the exact name of the rendering function above
    SketchCallback callback = (SketchCallback)&MjpegPlayDocoder::tft_output; // 强制转换func()的类型
    TJpgDec.setCallback(callback);
    video_start();
}

MjpegPlayDocoder::~MjpegPlayDocoder(void)
{
    Serial.println(F("~MjpegPlayDocoder"));
    tft->setSwapBytes(m_tftSwapStatus);
    // 释放资源
    video_end();
}

bool MjpegPlayDocoder::video_start()
{
    if (m_isUseDMA)
    {
        m_displayBuf = (uint8_t *)malloc(MOVIE_BUFFER_SIZE);
        m_jpegBuf = (uint8_t *)malloc(JPEG_BUFFER_SIZE);
        m_displayBufWithDma[0] = (uint8_t *)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
        m_displayBufWithDma[1] = (uint8_t *)heap_caps_malloc(DMA_BUFFER_SIZE, MALLOC_CAP_DMA);
        tft->initDMA();
        // 使用DMA
        // DMADrawer::setup(MOVIE_BUFFER_SIZE, SPI_FREQUENCY, TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC);
    }
    else
    {
        m_displayBuf = (uint8_t *)malloc(MOVIE_BUFFER_SIZE);
        tft->setAddrWindow((tft->width() - VIDEO_WIDTH) / 2,
                           (tft->height() - VIDEO_HEIGHT) / 2,
                           VIDEO_WIDTH, VIDEO_HEIGHT);
    }
    return true;

    // Serial.print("Stack: ");
    // Serial.println(uxTaskGetStackHighWaterMark(NULL));

    // Serial.print("heap_caps_get_free_size(): ");
    // Serial.println((unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT));
    // Serial.print("heap_caps_get_largest_free_block() :");
    // Serial.println((unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    // Serial.print("heap_caps_dump_all() :");
    // heap_caps_dump_all();

    // Serial.println("ESP.getFreeHeap()--------->");
    // Serial.println(ESP.getFreeHeap());

    // Serial.println(ESP.getFreeHeap());
    // Serial.println("<---------ESP.getFreeHeap()");
    // Serial.print("heap_caps_get_free_size(): ");
    // Serial.println((unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT));
    // Serial.print("heap_caps_get_largest_free_block() :");
    // Serial.println((unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

bool MjpegPlayDocoder::video_play_screen(void)
{
    // Read video
    uint32_t l = 0;

    if (m_isUseDMA)
    {
        // 一帧数据大概3000B 240M主频时花费50ms  80M时需要150ms
        // unsigned long Millis_1 = GET_SYS_MILLIS(); // 更新的时间
        uint32_t jpg_size = readJpegFromFile(m_pFile);
        // Serial.println(jpg_size);
        // Serial.print(GET_SYS_MILLIS() - Millis_1);
        // Serial.print(" ");
        // Millis_1 = GET_SYS_MILLIS();
        // Draw the image, top left at 0,0 - DMA request is handled in the call-back tft_output() in this sketch
        TJpgDec.drawJpg(0, 0, m_jpegBuf, jpg_size);
        // Serial.println(GET_SYS_MILLIS() - Millis_1);
    }
    else
    {
    }
    return true;
}

bool MjpegPlayDocoder::video_end(void)
{
    m_pFile = NULL;
    // 结束播放 释放资源
    if (NULL != m_displayBufWithDma[0])
    {
        free(m_displayBufWithDma[0]);
        free(m_displayBufWithDma[1]);
        m_displayBufWithDma[0] = NULL;
        m_displayBufWithDma[1] = NULL;
    }
    if (NULL != m_jpegBuf)
    {
        free(m_jpegBuf);
        m_jpegBuf = NULL;
    }
    // 需要添加wait 不然强行释放dma 会导致下一次initDMA失败
    // tft->dmaWait();
    // tft->deInitDMA();

    // 使用DMA
    // DMADrawer::setup(MOVIE_BUFFER_SIZE,
    //                  SPI_FREQUENCY,
    //                  TFT_MOSI, TFT_MISO,
    //                  TFT_SCLK, TFT_CS,
    //                  TFT_DC);
    // DMADrawer::close();
    if (NULL != m_displayBuf)
    {
        free(m_displayBuf);
        m_displayBuf = NULL;
    }

    return true;
}