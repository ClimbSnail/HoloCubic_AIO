/*
 * start rewrite from:
 * https://github.com/lovyan03/LovyanGFX/blob/master/src/lgfx/v0/platforms/LGFX_PARALLEL_ESP32.hpp
 */
#ifdef ESP32

#include "Arduino_DataBus.h"
#include "Arduino_ESP32I2S8.h"

// #define SET_DATA_AND_WR_AT_THE_SAME_TIME

Arduino_ESP32I2S8::Arduino_ESP32I2S8(
    int8_t dc, int8_t cs, int8_t wr, int8_t rd,
    int8_t d0, int8_t d1, int8_t d2, int8_t d3, int8_t d4, int8_t d5, int8_t d6, int8_t d7)
    : _dc(dc), _cs(cs), _wr(wr), _rd(rd),
      _d0(d0), _d1(d1), _d2(d2), _d3(d3), _d4(d4), _d5(d5), _d6(d6), _d7(d7)
{
}

void Arduino_ESP32I2S8::begin(int32_t speed, int8_t dataMode)
{
  pinMode(_dc, OUTPUT);
  digitalWrite(_dc, HIGH);

  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // disable chip select
  }
  if (_cs >= 32)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _csPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _csPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }

  pinMode(_wr, OUTPUT);
  digitalWrite(_wr, HIGH);

  if (_rd >= 0)
  {
    pinMode(_rd, OUTPUT);
    digitalWrite(_rd, HIGH);
  }

  gpio_pad_select_gpio(_d0);
  gpio_pad_select_gpio(_d1);
  gpio_pad_select_gpio(_d2);
  gpio_pad_select_gpio(_d3);
  gpio_pad_select_gpio(_d4);
  gpio_pad_select_gpio(_d5);
  gpio_pad_select_gpio(_d6);
  gpio_pad_select_gpio(_d7);

  auto idx_base = (_i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
  gpio_matrix_out(_dc, idx_base + 8, 0, 0);
  gpio_matrix_out(_d7, idx_base + 7, 0, 0);
  gpio_matrix_out(_d6, idx_base + 6, 0, 0);
  gpio_matrix_out(_d5, idx_base + 5, 0, 0);
  gpio_matrix_out(_d4, idx_base + 4, 0, 0);
  gpio_matrix_out(_d3, idx_base + 3, 0, 0);
  gpio_matrix_out(_d2, idx_base + 2, 0, 0);
  gpio_matrix_out(_d1, idx_base + 1, 0, 0);
  gpio_matrix_out(_d0, idx_base, 0, 0);

  uint32_t dport_clk_en;
  uint32_t dport_rst;

  if (_i2s_port == I2S_NUM_0)
  {
    idx_base = I2S0O_WS_OUT_IDX;
    dport_clk_en = DPORT_I2S0_CLK_EN;
    dport_rst = DPORT_I2S0_RST;
  }
  else
  {
    idx_base = I2S1O_WS_OUT_IDX;
    dport_clk_en = DPORT_I2S1_CLK_EN;
    dport_rst = DPORT_I2S1_RST;
  }
  gpio_matrix_out(_wr, idx_base, 1, 0); // WR (Write-strobe in 8080 mode, Active-low)

  DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, dport_clk_en);
  DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, dport_rst);

  //Reset I2S subsystem
  *reg(I2S_CONF_REG(_i2s_port)) = I2S_TX_RESET | I2S_RX_RESET | I2S_TX_FIFO_RESET | I2S_RX_FIFO_RESET;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_default;

  //Reset DMA
  *reg(I2S_LC_CONF_REG(_i2s_port)) = I2S_IN_RST | I2S_OUT_RST | I2S_AHBM_RST | I2S_AHBM_FIFO_RST;
  *reg(I2S_LC_CONF_REG(_i2s_port)) = I2S_OUT_EOF_MODE;

  *reg(I2S_CONF2_REG(_i2s_port)) = I2S_LCD_EN;

  *reg(I2S_CONF1_REG(_i2s_port)) = I2S_TX_PCM_BYPASS | I2S_TX_STOP_EN;

  *reg(I2S_CONF_CHAN_REG(_i2s_port)) = 1 << I2S_TX_CHAN_MOD_S | 1 << I2S_RX_CHAN_MOD_S;

  *reg(I2S_INT_ENA_REG(_i2s_port)) |= I2S_TX_REMPTY_INT_ENA;

  *reg(I2S_OUT_LINK_REG(_i2s_port)) = 0;
  *reg(I2S_IN_LINK_REG(_i2s_port)) = 0;
  *reg(I2S_TIMING_REG(_i2s_port)) = 0;

  uint32_t apb_freq = getApbFrequency();
  // clock = 80MHz(apb_freq) / I2S_CLKM_DIV_NUM
  // I2S_CLKM_DIV_NUM 4=20MHz  /  5=16MHz  /  8=10MHz  /  10=8MHz
  uint32_t div_num = min(32ul, max(4ul, 1 + (apb_freq / (1 + 20000000ul))));
  _clkdiv_write = I2S_CLKA_ENA | I2S_CLK_EN | 1 << I2S_CLKM_DIV_A_S | 0 << I2S_CLKM_DIV_B_S | div_num << I2S_CLKM_DIV_NUM_S;
}

void Arduino_ESP32I2S8::beginWrite()
{
  *reg(I2S_CLKM_CONF_REG(_i2s_port)) = _clkdiv_write;

  CS_LOW();
}

void Arduino_ESP32I2S8::endWrite()
{
  if (_cs < 0)
  {
    writeCommand(0); // NOP command
  }
  wait_i2s();
  CS_HIGH();
}

void Arduino_ESP32I2S8::writeCommand(uint8_t c)
{
  wait();
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = c << 16;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::writeCommand16(uint16_t c)
{
  wait();
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ((c & 0xff00) << 8) | (c & 0xff);
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::write(uint8_t d)
{
  wait();
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = (0x0100 | d) << 16;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::write16(uint16_t d)
{
  wait();
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = (d << 8) | d | 0x01000100;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::writeRepeat(uint16_t p, uint32_t len)
{
  auto conf_start = _conf_reg_start;
  uint32_t data = 0x01000100 | p << 8 | p;
  int32_t limit = ((len - 1) & 31) + 1;
  do
  {
    len -= limit;

    wait();
    *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
    *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

    while (limit--)
    {
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data;
    }
    *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
    limit = 32;
  } while (len);
}

void Arduino_ESP32I2S8::writePixels(uint16_t *data, uint32_t len)
{
  auto conf_start = _conf_reg_start;
  static constexpr uint32_t data_wr = 0x01000100;
  int32_t limit = ((len - 1) & 31) + 1;
  do
  {
    len -= limit;

    wait();
    *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
    *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
    while (limit--)
    {
      _data16.value = *data++;
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = _data16.msb << 16 | _data16.lsb | data_wr;
    }
    *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
    limit = 32;
  } while (len);
}

void Arduino_ESP32I2S8::writeC8D8(uint8_t c, uint8_t d)
{
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
  wait_i2s();
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = c;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = (0x0100 | d) << 16;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::writeC8D16(uint8_t c, uint16_t d)
{
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
  static constexpr uint32_t data_wr = 0x01000100;
  wait_i2s();
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = c;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = d << 8 | d | data_wr;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2)
{
  *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
  static constexpr uint32_t data_wr = 0x01000100;
  wait_i2s();
  *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = c;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = d1 << 8 | d1 | data_wr;
  *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = d2 << 8 | d2 | data_wr;
  *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
}

void Arduino_ESP32I2S8::writeBytes(uint8_t *data, uint32_t len)
{
  if (len & 1)
  {
    write(data[0]);
    if (!--len)
      return;
    ++data;
  }

  auto conf_start = _conf_reg_start;
  static constexpr uint32_t data_wr = 0x01000100;
  int32_t limit = ((len - 1) & 31) + 1;
  do
  {
    len -= limit << 1;

    wait();
    *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
    *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
    while (limit--)
    {
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data[0] << 16 | data[1] | data_wr;
      data += 2;
    }
    *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
    limit = 32;
  } while (len);
}

void Arduino_ESP32I2S8::writePattern(uint8_t *data, uint8_t len, uint32_t repeat)
{
  while (repeat--)
  {
    writeBytes(data, len);
  }
}

INLINE volatile uint32_t *Arduino_ESP32I2S8::reg(uint32_t addr) const
{
  return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr);
}

INLINE void Arduino_ESP32I2S8::wait_i2s(void) const
{
  auto conf_reg1 = _conf_reg_default | I2S_TX_RESET | I2S_RX_RESET | I2S_RX_FIFO_RESET;
  while (!(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE))
    ;
  *reg(I2S_CONF_REG(_i2s_port)) = conf_reg1;
}

INLINE void Arduino_ESP32I2S8::wait(void) const
{
  auto conf_reg1 = _conf_reg_default | I2S_TX_RESET | I2S_RX_RESET | I2S_RX_FIFO_RESET;

  while (!(*reg(I2S_INT_RAW_REG(_i2s_port)) & I2S_TX_REMPTY_INT_RAW))
    ;
  *reg(I2S_INT_CLR_REG(_i2s_port)) = I2S_TX_REMPTY_INT_CLR;

  while (!(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE))
    ;
  *reg(I2S_CONF_REG(_i2s_port)) = conf_reg1;
}

/******** low level bit twiddling **********/

INLINE void Arduino_ESP32I2S8::CS_HIGH(void)
{
  if (_cs >= 0)
  {
    *_csPortSet = _csPinMask;
  }
}

INLINE void Arduino_ESP32I2S8::CS_LOW(void)
{
  if (_cs >= 0)
  {
    *_csPortClr = _csPinMask;
  }
}

#endif
