#ifndef TFT_eSPI_STUB_H
#define TFT_eSPI_STUB_H

#include <stdint.h>

#define TFT_BLACK 0x0000
#define ST7789_DISPON 0x29

class TFT_eSPI
{
public:
    TFT_eSPI() : _textsize(1), _cursor_x(0), _cursor_y(0),
                 _textcolor(0xFFFF), _textbgcolor(0x0000),
                 _fg_color(0xFFFF), _bg_color(0x0000),
                 _win_x(0), _win_y(0), _win_w(0), _win_h(0), _win_ptr(0) {}
    TFT_eSPI(int16_t w, int16_t h) : TFT_eSPI() { (void)w; (void)h; }

    void begin(void) {}

    // 屏幕尺寸
    int32_t width()  { return 240; }
    int32_t height() { return 240; }

    // 颜色转换
    uint32_t color565(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)(r & 0xF8) << 8) | ((uint32_t)(g & 0xFC) << 3) | (b >> 3);
    }

    // 绘制函数（全部实现在 TFT_eSPI.cpp，直接写入 fb_buf）
    void fillScreen(uint32_t color);
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

    // 文本渲染
    void setTextColor(uint32_t color)         { _textcolor = color; }
    void setTextColor(uint32_t fg, uint32_t bg) { _fg_color = fg; _bg_color = bg; _textcolor = fg; _textbgcolor = bg; }
    void setTextSize(uint8_t s)               { _textsize = s; }
    void setCursor(int32_t x, int32_t y)      { _cursor_x = x; _cursor_y = y; }
    void drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size);
    void drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font);
    void print(const char *s);
    void print(int n);
    void println(const char *s);
    void println(int n);

    // 图像推送
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data);
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data);

    // 以下保持空实现（当前仿真不需要）
    void writecommand(uint8_t cmd) { (void)cmd; }
    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h);
    void startWrite(void) {}
    void pushColors(const void *data, uint32_t len, bool swap);
    void pushColors(const void *data, uint32_t len) { pushColors(data, len, false); }
    void endWrite(void) {}
    void setRotation(uint8_t r) { (void)r; }
    uint8_t readcommand8(uint8_t cmd, uint8_t index) { (void)cmd; (void)index; return 0; }
    void pushPixels(const void *data, uint32_t len);
    bool getSwapBytes(void) { return false; }
    void setSwapBytes(bool swap) { (void)swap; }
    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *bitmap, uint16_t *dmaBuffer);
    void initDMA(void) {}

    // 文本状态（公开，供 drawChar 内部使用）
    uint8_t  _textsize;
    int32_t  _cursor_x;
    int32_t  _cursor_y;
    uint32_t _textcolor;
    uint32_t _textbgcolor;
    uint32_t _fg_color;
    uint32_t _bg_color;

private:
    // 写窗口状态（setAddrWindow → pushColors/pushPixels 的上下文）
    int32_t _win_x, _win_y, _win_w, _win_h;
    int32_t _win_ptr;
};

extern TFT_eSPI *tft;

#endif /* TFT_eSPI_STUB_H */