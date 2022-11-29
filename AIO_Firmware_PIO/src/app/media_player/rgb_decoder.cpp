#include "docoder.h"
#include "common.h"

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define MOVIE_BUFFER_SIZE 28800 // (57600)

#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises

RgbPlayDocoder::RgbPlayDocoder(File *file, bool isUseDMA)
{
    m_pFile = file;
    m_isUseDMA = isUseDMA;
    m_displayBuf = NULL;
    m_displayBufWithDma[0] = NULL;
    m_displayBufWithDma[1] = NULL;
    video_start();
}

RgbPlayDocoder::~RgbPlayDocoder(void)
{
    Serial.println(F("~RgbPlayDocoder"));
    // 释放资源
    video_end();
}

bool RgbPlayDocoder::video_start()
{
    if (m_isUseDMA)
    {
        m_displayBufWithDma[0] = (uint8_t *)heap_caps_malloc(MOVIE_BUFFER_SIZE, MALLOC_CAP_DMA);
        m_displayBufWithDma[1] = (uint8_t *)heap_caps_malloc(MOVIE_BUFFER_SIZE, MALLOC_CAP_DMA);
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

bool RgbPlayDocoder::video_play_screen(void)
{
    // Read video
    uint32_t l = 0;
    unsigned long Millis_1 = 0; // 更新的时间

    if (m_isUseDMA)
    {
        // 80M主频大概200ms一帧 240M大概150ms一帧
        uint8_t *dst = NULL;
        dst = m_displayBufWithDma[0];
        m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        tft->pushImageDMA(0, 0, 240, 60, (uint16_t *)dst, nullptr);

        dst = m_displayBufWithDma[1];
        m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        tft->pushImageDMA(0, 60, 240, 60, (uint16_t *)dst, nullptr);

        dst = m_displayBufWithDma[0];
        m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        tft->pushImageDMA(0, 120, 240, 60, (uint16_t *)dst, nullptr);

        dst = m_displayBufWithDma[1];
        m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        tft->pushImageDMA(0, 180, 240, 60, (uint16_t *)dst, nullptr);

        // 以下是使用DMADrawer接口的实现 目前有一定问题，暂时放着
        // uint8_t *dst = NULL;
        // dst = DMADrawer::getNextBuffer();
        // l = m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        // DMADrawer::draw(0, 0, 240, 60);

        // dst = DMADrawer::getNextBuffer();
        // l = m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        // DMADrawer::draw(0, 60, 240, 60);

        // dst = DMADrawer::getNextBuffer();
        // l = m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        // DMADrawer::draw(0, 120, 240, 60);

        // dst = DMADrawer::getNextBuffer();
        // l = m_pFile->read(dst, MOVIE_BUFFER_SIZE);
        // DMADrawer::draw(0, 180, 240, 60);
    }
    else
    {

        tft->startWrite();
        Millis_1 = GET_SYS_MILLIS();
        l = m_pFile->read(m_displayBuf, MOVIE_BUFFER_SIZE);
        Serial.println(GET_SYS_MILLIS() - Millis_1);
        Millis_1 = GET_SYS_MILLIS();
        tft->pushColors(m_displayBuf, l);
        Serial.println(GET_SYS_MILLIS() - Millis_1);
        Millis_1 = GET_SYS_MILLIS();
        l = m_pFile->read(m_displayBuf, MOVIE_BUFFER_SIZE);
        Serial.println(GET_SYS_MILLIS() - Millis_1);
        Millis_1 = GET_SYS_MILLIS();
        tft->pushColors(m_displayBuf, l);
        Serial.println(GET_SYS_MILLIS() - Millis_1);

        l = m_pFile->read(m_displayBuf, MOVIE_BUFFER_SIZE);
        tft->pushColors(m_displayBuf, l);
        l = m_pFile->read(m_displayBuf, MOVIE_BUFFER_SIZE);
        tft->pushColors(m_displayBuf, l);

        tft->endWrite();
    }
    return true;
}

bool RgbPlayDocoder::video_end(void)
{
    m_pFile = NULL;
    // 结束播放 释放资源
    if (m_isUseDMA)
    {
        if (NULL != m_displayBufWithDma[0])
        {
            free(m_displayBufWithDma[0]);
            free(m_displayBufWithDma[1]);
            m_displayBufWithDma[0] = NULL;
            m_displayBufWithDma[1] = NULL;
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
    }
    else
    {
        if (NULL != m_displayBuf)
        {
            free(m_displayBuf);
            m_displayBuf = NULL;
        }
    }

    return true;
}