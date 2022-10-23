#ifndef PLAYER_H
#define PLAYER_H

#include <SD.h>

class PlayDocoderBase
{
public:
    virtual ~PlayDocoderBase(){};
    virtual bool video_start() { return true; };
    virtual bool video_play_screen() { return true; };
    virtual bool video_end() { return true; };
};

class RgbPlayDocoder : public PlayDocoderBase
{
private:
    File *m_pFile;
    bool m_isUseDMA;
    uint8_t *m_displayBuf;
    uint8_t *m_displayBufWithDma[2];

public:
    RgbPlayDocoder(File *file, bool isUseDMA = false);
    virtual ~RgbPlayDocoder();
    virtual bool video_start();
    virtual bool video_play_screen();
    virtual bool video_end();
};

class MjpegPlayDocoder : public PlayDocoderBase
{
public:
    File *m_pFile;
    static bool m_isUseDMA; // 是否使用DMA
    uint8_t *m_displayBuf;  // 显示的
    int32_t m_bufSaveTail;  // 指向 m_displayBuf 中所保存的最后一个数据所在下标
    uint8_t *m_jpegBuf;     // 用来给 jpeg 图片做缓冲，将此提交给jpeg解码器解码
    bool m_tftSwapStatus;   // 由于jpeg图片解码后需要互换高低位才可以使用tft_espi进行显示
    // 由此保存环境当前的高低位置换，以便退出视频播放的时候还原回去。
    static uint8_t *m_displayBufWithDma[2];
    static bool m_dmaBufferSel;

public:
    MjpegPlayDocoder(File *file, bool isUseDMA = false);
    virtual ~MjpegPlayDocoder();
    uint32_t readJpegFromFile(File *file);
    bool static tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
    virtual bool video_start();
    virtual bool video_play_screen();
    virtual bool video_end();
};

#endif