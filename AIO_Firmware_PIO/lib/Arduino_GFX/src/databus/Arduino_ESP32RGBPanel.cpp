#include "Arduino_ESP32RGBPanel.h"

#if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

Arduino_ESP32RGBPanel::Arduino_ESP32RGBPanel(
    int8_t cs, int8_t sck, int8_t sda,
    int8_t de, int8_t vsync, int8_t hsync, int8_t pclk,
    int8_t r0, int8_t r1, int8_t r2, int8_t r3, int8_t r4,
    int8_t g0, int8_t g1, int8_t g2, int8_t g3, int8_t g4, int8_t g5,
    int8_t b0, int8_t b1, int8_t b2, int8_t b3, int8_t b4)
    : _cs(cs), _sck(sck), _sda(sda),
      _de(de), _vsync(vsync), _hsync(hsync), _pclk(pclk),
      _r0(r0), _r1(r1), _r2(r2), _r3(r3), _r4(r4),
      _g0(g0), _g1(g1), _g2(g2), _g3(g3), _g4(g4), _g5(g5),
      _b0(b0), _b1(b1), _b2(b2), _b3(b3), _b4(b4)
{
}

void Arduino_ESP32RGBPanel::begin(int32_t speed)
{
  _speed = speed;

  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH); // Deselect
  pinMode(_sck, OUTPUT);
  digitalWrite(_sck, LOW);
  pinMode(_sda, OUTPUT);
  digitalWrite(_sda, LOW);

  if (_cs >= 32)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _csPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _csPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  if (_sck >= 32)
  {
    _sckPinMask = digitalPinToBitMask(_sck);
    _sckPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _sckPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _sckPinMask = digitalPinToBitMask(_sck);
    _sckPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _sckPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  if (_sda >= 32)
  {
    _sdaPinMask = digitalPinToBitMask(_sda);
    _sdaPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _sdaPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _sdaPinMask = digitalPinToBitMask(_sda);
    _sdaPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _sdaPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
}

void Arduino_ESP32RGBPanel::sendCommand(uint8_t c)
{
  CS_LOW();

  // D/C bit, command
  SDA_LOW();
  SCK_HIGH();
  SCK_LOW();

  uint8_t bit = 0x80;
  while (bit)
  {
    if (c & bit)
    {
      SDA_HIGH();
    }
    else
    {
      SDA_LOW();
    }
    SCK_HIGH();
    bit >>= 1;
    SCK_LOW();
  }

  CS_HIGH();
}

void Arduino_ESP32RGBPanel::sendData(uint8_t d)
{
  CS_LOW();

  // D/C bit, data
  SDA_HIGH();
  SCK_HIGH();
  SCK_LOW();

  uint8_t bit = 0x80;
  while (bit)
  {
    if (d & bit)
    {
      SDA_HIGH();
    }
    else
    {
      SDA_LOW();
    }
    SCK_HIGH();
    bit >>= 1;
    SCK_LOW();
  }

  CS_HIGH();
}

uint16_t *Arduino_ESP32RGBPanel::getFrameBuffer(int16_t w, int16_t h)
{
  esp_lcd_rgb_panel_config_t *_panel_config = (esp_lcd_rgb_panel_config_t *)heap_caps_calloc(1, sizeof(esp_lcd_rgb_panel_config_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

  _panel_config->data_width = 16; // RGB565 in parallel mode, thus 16bit in width
  _panel_config->psram_trans_align = 64;
  _panel_config->clk_src = LCD_CLK_SRC_PLL160M;
  _panel_config->disp_gpio_num = GPIO_NUM_NC;
  _panel_config->pclk_gpio_num = _pclk;
  _panel_config->vsync_gpio_num = _vsync;
  _panel_config->hsync_gpio_num = _hsync;
  _panel_config->de_gpio_num = _de;
  // _panel_config->data_gpio_nums[0] = _b0;
  // _panel_config->data_gpio_nums[1] = _b1;
  // _panel_config->data_gpio_nums[2] = _b2;
  // _panel_config->data_gpio_nums[3] = _b3;
  // _panel_config->data_gpio_nums[4] = _b4;
  // _panel_config->data_gpio_nums[5] = _g0;
  // _panel_config->data_gpio_nums[6] = _g1;
  // _panel_config->data_gpio_nums[7] = _g2;
  // _panel_config->data_gpio_nums[8] = _g3;
  // _panel_config->data_gpio_nums[9] = _g4;
  // _panel_config->data_gpio_nums[10] = _g5;
  // _panel_config->data_gpio_nums[11] = _r0;
  // _panel_config->data_gpio_nums[12] = _r1;
  // _panel_config->data_gpio_nums[13] = _r2;
  // _panel_config->data_gpio_nums[14] = _r3;
  // _panel_config->data_gpio_nums[15] = _r4;

  // Big Endian Pixel format
  _panel_config->data_gpio_nums[0] = _g3;
  _panel_config->data_gpio_nums[1] = _g4;
  _panel_config->data_gpio_nums[2] = _g5;
  _panel_config->data_gpio_nums[3] = _r0;
  _panel_config->data_gpio_nums[4] = _r1;
  _panel_config->data_gpio_nums[5] = _r2;
  _panel_config->data_gpio_nums[6] = _r3;
  _panel_config->data_gpio_nums[7] = _r4;
  _panel_config->data_gpio_nums[8] = _b0;
  _panel_config->data_gpio_nums[9] = _b1;
  _panel_config->data_gpio_nums[10] = _b2;
  _panel_config->data_gpio_nums[11] = _b3;
  _panel_config->data_gpio_nums[12] = _b4;
  _panel_config->data_gpio_nums[13] = _g0;
  _panel_config->data_gpio_nums[14] = _g1;
  _panel_config->data_gpio_nums[15] = _g2;

  _panel_config->timings.pclk_hz = _speed;
  _panel_config->timings.h_res = w;
  _panel_config->timings.v_res = h;
  // The following parameters should refer to LCD spec
  _panel_config->timings.hsync_back_porch = 50;
  _panel_config->timings.hsync_front_porch = 10;
  _panel_config->timings.hsync_pulse_width = 8;
  _panel_config->timings.vsync_back_porch = 20;
  _panel_config->timings.vsync_front_porch = 10;
  _panel_config->timings.vsync_pulse_width = 8;
  _panel_config->timings.flags.pclk_active_neg = 0; // RGB data is clocked out on falling edge
  _panel_config->flags.fb_in_psram = 1;             // allocate frame buffer in PSRAM
  _panel_config->flags.relax_on_idle = 0;

  ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(_panel_config, &_panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(_panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(_panel_handle));

  uint16_t color = random(0xffff);
  ESP_ERROR_CHECK(_panel_handle->draw_bitmap(_panel_handle, 0, 0, 1, 1, &color));

  _rgb_panel = __containerof(_panel_handle, esp_rgb_panel_t, base);

  return (uint16_t *)_rgb_panel->fb;
}

INLINE void Arduino_ESP32RGBPanel::CS_HIGH(void)
{
  *_csPortSet = _csPinMask;
}

INLINE void Arduino_ESP32RGBPanel::CS_LOW(void)
{
  *_csPortClr = _csPinMask;
}

INLINE void Arduino_ESP32RGBPanel::SCK_HIGH(void)
{
  *_sckPortSet = _sckPinMask;
}

INLINE void Arduino_ESP32RGBPanel::SCK_LOW(void)
{
  *_sckPortClr = _sckPinMask;
}

INLINE void Arduino_ESP32RGBPanel::SDA_HIGH(void)
{
  *_sdaPortSet = _sdaPinMask;
}

INLINE void Arduino_ESP32RGBPanel::SDA_LOW(void)
{
  *_sdaPortClr = _sdaPinMask;
}
#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)
