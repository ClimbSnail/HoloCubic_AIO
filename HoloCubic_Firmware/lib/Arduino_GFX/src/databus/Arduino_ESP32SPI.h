/*
 * start rewrite from:
 * https://github.com/espressif/arduino-esp32.git
 */
#ifdef ESP32

#ifndef _ARDUINO_ESP32SPI_H_
#define _ARDUINO_ESP32SPI_H_

#include "soc/spi_struct.h"
#include "soc/dport_reg.h"

#include "Arduino_DataBus.h"

#define SPI_MAX_PIXELS_AT_ONCE 32

#define SPI_SPI_SS_IDX(n) ((n == 0) ? SPICS0_OUT_IDX : ((n == 1) ? SPICS1_OUT_IDX : ((n == 2) ? SPICS2_OUT_IDX : SPICS0_OUT_IDX)))
#define SPI_HSPI_SS_IDX(n) ((n == 0) ? HSPICS0_OUT_IDX : ((n == 1) ? HSPICS1_OUT_IDX : ((n == 2) ? HSPICS2_OUT_IDX : HSPICS0_OUT_IDX)))
#define SPI_VSPI_SS_IDX(n) ((n == 0) ? VSPICS0_OUT_IDX : ((n == 1) ? VSPICS1_OUT_IDX : ((n == 2) ? VSPICS2_OUT_IDX : VSPICS0_OUT_IDX)))
#define SPI_SS_IDX(p, n) ((p == 0) ? SPI_SPI_SS_IDX(n) : ((p == 1) ? SPI_SPI_SS_IDX(n) : ((p == 2) ? SPI_HSPI_SS_IDX(n) : ((p == 3) ? SPI_VSPI_SS_IDX(n) : 0))))

class Arduino_ESP32SPI : public Arduino_DataBus
{
public:
  Arduino_ESP32SPI(int8_t dc = -1, int8_t cs = -1, int8_t sck = -1, int8_t mosi = -1, int8_t miso = -1, uint8_t spi_num = VSPI, bool is_shared_interface = true); // Constructor

  void begin(int32_t speed = 0, int8_t dataMode = SPI_MODE0) override;
  void beginWrite() override;
  void endWrite() override;
  void writeCommand(uint8_t) override;
  void writeCommand16(uint16_t) override;
  void write(uint8_t) override;
  void write16(uint16_t) override;
  void writeRepeat(uint16_t p, uint32_t len) override;
  void writePixels(uint16_t *data, uint32_t len) override;

  void writeC8D8(uint8_t c, uint8_t d) override;
  void writeC8D16(uint8_t c, uint16_t d) override;
  void writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2) override;
  void writeBytes(uint8_t *data, uint32_t len) override;
  void writePattern(uint8_t *data, uint8_t len, uint32_t repeat) override;

  void writeIndexedPixels(uint8_t *data, uint16_t *idx, uint32_t len) override;
  void writeIndexedPixelsDouble(uint8_t *data, uint16_t *idx, uint32_t len) override;

protected:
  void flush_data_buf();

private:
  INLINE void WRITE8BIT(uint8_t c);
  INLINE void WRITE9BIT(uint32_t c);
  INLINE void DC_HIGH(void);
  INLINE void DC_LOW(void);
  INLINE void CS_HIGH(void);
  INLINE void CS_LOW(void);

  int8_t _dc, _cs;
  int8_t _sck, _mosi, _miso;
  uint8_t _spi_num;
  bool _is_shared_interface;
  int32_t _speed;
  uint32_t _div = 0;

  PORTreg_t _dcPortSet; ///< PORT register for data/command SET
  PORTreg_t _dcPortClr; ///< PORT register for data/command CLEAR
  PORTreg_t _csPortSet; ///< PORT register for chip select SET
  PORTreg_t _csPortClr; ///< PORT register for chip select CLEAR
  uint32_t _dcPinMask;  ///< Bitmask for data/command
  uint32_t _csPinMask;  ///< Bitmask for chip select

  spi_t *_spi;
  uint8_t _bitOrder = SPI_MSBFIRST;
  union
  {
    uint8_t _buffer[SPI_MAX_PIXELS_AT_ONCE * 2] = {0};
    uint32_t _buffer32[SPI_MAX_PIXELS_AT_ONCE / 2];
  };
  uint16_t _data_buf_bit_idx = 0;
};

#endif // _ARDUINO_ESP32SPI_H_

#endif // #ifdef ESP32
