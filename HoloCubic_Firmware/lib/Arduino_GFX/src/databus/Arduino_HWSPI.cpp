/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include <SPI.h>
#include "Arduino_HWSPI.h"

#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#define HWSPI LCD_SPI
#elif defined(RTL8722DM)
#define HWSPI SPI1
#else
#define HWSPI SPI
#endif

#if defined(SPI_HAS_TRANSACTION)
#define SPI_BEGIN_TRANSACTION() HWSPI.beginTransaction(mySPISettings)
#define SPI_END_TRANSACTION() HWSPI.endTransaction()
#else
#define SPI_BEGIN_TRANSACTION() \
  {                             \
  }
#define SPI_END_TRANSACTION() \
  {                           \
  }
#endif

#if defined(SPI_HAS_TRANSACTION)
static SPISettings mySPISettings;
#elif defined(__AVR__) || defined(CORE_TEENSY)
static uint8_t SPCRbackup;
static uint8_t mySPCR;
#endif

#if defined(ESP32)
Arduino_HWSPI::Arduino_HWSPI(int8_t dc, int8_t cs /* = -1 */, int8_t sck /* = -1 */, int8_t mosi /* = -1 */, int8_t miso /* = -1 */, bool is_shared_interface /* = true */)
    : _dc(dc), _cs(cs), _sck(sck), _mosi(mosi), _miso(miso), _is_shared_interface(is_shared_interface)
{
#else
Arduino_HWSPI::Arduino_HWSPI(int8_t dc, int8_t cs /* = -1 */, bool is_shared_interface /* = true */)
    : _dc(dc), _cs(cs), _is_shared_interface(is_shared_interface)
{
#endif
}

void Arduino_HWSPI::begin(int32_t speed, int8_t dataMode)
{
  _speed = speed ? speed : SPI_DEFAULT_FREQ;
  _dataMode = dataMode;

  pinMode(_dc, OUTPUT);
  digitalWrite(_dc, HIGH); // Data mode
  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // Deselect
  }

#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(ARDUINO_ARCH_NRF52840)
  uint32_t pin = digitalPinToPinName((pin_size_t)_dc);
  NRF_GPIO_Type *reg = nrf_gpio_pin_port_decode(&pin);
  nrf_gpio_cfg_output(pin);
  _dcPortSet = &reg->OUTSET;
  _dcPortClr = &reg->OUTCLR;
  _dcPinMask = 1UL << pin;
  if (_cs >= 0)
  {
    pin = digitalPinToPinName((pin_size_t)_cs);
    reg = nrf_gpio_pin_port_decode(&pin);
    nrf_gpio_cfg_output(pin);
    _csPortSet = &reg->OUTSET;
    _csPortClr = &reg->OUTCLR;
    _csPinMask = 1UL << pin;
  }
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  _dcPinMask = digitalPinToBitMask(_dc);
  _dcPortSet = (PORTreg_t)&sio_hw->gpio_set;
  _dcPortClr = (PORTreg_t)&sio_hw->gpio_clr;
  if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&sio_hw->gpio_set;
    _csPortClr = (PORTreg_t)&sio_hw->gpio_clr;
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = (PORTreg_t)_dcPortSet;
    _csPortClr = (PORTreg_t)_dcPortClr;
    _csPinMask = 0;
  }
#elif defined(ESP32)
  _dcPinMask = digitalPinToBitMask(_dc);
  if (_dc >= 32)
  {
    _dcPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _dcPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _dcPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _dcPortClr = (PORTreg_t)&GPIO.out_w1tc;
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
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = (PORTreg_t)_dcPortSet;
    _csPortClr = (PORTreg_t)_dcPortClr;
    _csPinMask = 0;
  }
#elif defined(CORE_TEENSY)
#if !defined(KINETISK)
  _dcPinMask = digitalPinToBitMask(_dc);
#endif
  _dcPortSet = portSetRegister(_dc);
  _dcPortClr = portClearRegister(_dc);
  if (_cs >= 0)
  {
#if !defined(KINETISK)
    _csPinMask = digitalPinToBitMask(_cs);
#endif
    _csPortSet = portSetRegister(_cs);
    _csPortClr = portClearRegister(_cs);
  }
  else
  {
#if !defined(KINETISK)
    _csPinMask = 0;
#endif
    _csPortSet = _dcPortSet;
    _csPortClr = _dcPortClr;
  }
#else  // !CORE_TEENSY
  _dcPinMask = digitalPinToBitMask(_dc);
  _dcPortSet = &(PORT->Group[g_APinDescription[_dc].ulPort].OUTSET.reg);
  _dcPortClr = &(PORT->Group[g_APinDescription[_dc].ulPort].OUTCLR.reg);
  if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = &(PORT->Group[g_APinDescription[_cs].ulPort].OUTSET.reg);
    _csPortClr = &(PORT->Group[g_APinDescription[_cs].ulPort].OUTCLR.reg);
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = _dcPortSet;
    _csPortClr = _dcPortClr;
    _csPinMask = 0;
  }
#endif // end !CORE_TEENSY
#else  // !HAS_PORT_SET_CLR
  _dcPort = (PORTreg_t)portOutputRegister(digitalPinToPort(_dc));
  _dcPinMaskSet = digitalPinToBitMask(_dc);
  if (_cs >= 0)
  {
    _csPort = (PORTreg_t)portOutputRegister(digitalPinToPort(_cs));
    _csPinMaskSet = digitalPinToBitMask(_cs);
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPort = _dcPort;
    _csPinMaskSet = 0;
  }
  _csPinMaskClr = ~_csPinMaskSet;
  _dcPinMaskClr = ~_dcPinMaskSet;
#endif // !HAS_PORT_SET_CLR
#endif // USE_FAST_PINIO

#if defined(ESP32)
  HWSPI.begin(_sck, _miso, _mosi);
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE0;
  }
  mySPISettings = SPISettings(_speed, MSBFIRST, _dataMode);
#elif defined(ESP8266)
  HWSPI.begin();
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE0;
  }
  mySPISettings = SPISettings(_speed, MSBFIRST, _dataMode);
// Teensy 4.x
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)
  HWSPI.begin();
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE0;
  }
  mySPISettings = SPISettings(_speed, MSBFIRST, _dataMode);
#elif defined(SPI_HAS_TRANSACTION)
  HWSPI.begin();
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE2;
  }
  mySPISettings = SPISettings(_speed, MSBFIRST, _dataMode);
#elif defined(__AVR__) || defined(CORE_TEENSY)
  SPCRbackup = SPCR;
  HWSPI.begin();
  HWSPI.setClockDivider(SPI_CLOCK_DIV2);
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE2;
  }
  HWSPI.setDataMode(_dataMode);
  mySPCR = SPCR;     // save our preferred state
  SPCR = SPCRbackup; // then restore
#elif defined(__SAM3X8E__)
  HWSPI.begin();
  HWSPI.setClockDivider(21); //4MHz
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE2;
  }
  HWSPI.setDataMode(_dataMode);
#elif defined(__arm__)
  if (_dataMode < 0)
  {
    _dataMode = SPI_MODE2;
  }
#endif
}

void Arduino_HWSPI::beginWrite()
{
  if (_is_shared_interface)
  {
    SPI_BEGIN_TRANSACTION();
  }

  DC_HIGH();
  CS_LOW();
}

void Arduino_HWSPI::endWrite()
{
  CS_HIGH();

  if (_is_shared_interface)
  {
    SPI_END_TRANSACTION();
  }
}

void Arduino_HWSPI::writeCommand(uint8_t c)
{
  DC_LOW();

  WRITE(c);

  DC_HIGH();
}

void Arduino_HWSPI::writeCommand16(uint16_t c)
{
  DC_LOW();

#if defined(LITTLE_FOOT_PRINT)
  _data16.value = c;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);
#else  // !defined(LITTLE_FOOT_PRINT)
  WRITE16(c);
#endif // !defined(LITTLE_FOOT_PRINT)

  DC_HIGH();
}

void Arduino_HWSPI::write(uint8_t d)
{
  WRITE(d);
}

void Arduino_HWSPI::write16(uint16_t d)
{
#if defined(LITTLE_FOOT_PRINT)
  _data16.value = d;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);
#else  // !defined(LITTLE_FOOT_PRINT)
  WRITE16(d);
#endif // !defined(LITTLE_FOOT_PRINT)
}

void Arduino_HWSPI::writeRepeat(uint16_t p, uint32_t len)
{
#if defined(LITTLE_FOOT_PRINT)
  _data16.value = p;
  while (len--)
  {
    WRITE(_data16.msb);
    WRITE(_data16.lsb);
  }
#else  // !defined(LITTLE_FOOT_PRINT)
  MSB_16_SET(p, p);
  uint32_t xferLen;

  while (len)
  {
    xferLen = (len < SPI_MAX_PIXELS_AT_ONCE) ? len : SPI_MAX_PIXELS_AT_ONCE;
    for (uint32_t i = 0; i < xferLen; i++)
    {
      _buffer.v16[i] = p;
    }
    len -= xferLen;

    xferLen += xferLen;
    WRITEBUF(_buffer.v8, xferLen);
  }
#endif // !defined(LITTLE_FOOT_PRINT)
}

void Arduino_HWSPI::writePixels(uint16_t *data, uint32_t len)
{
#if defined(LITTLE_FOOT_PRINT)
  while (len--)
  {
    _data16.value = *data++;
    WRITE(_data16.msb);
    WRITE(_data16.lsb);
  }
#else  // !defined(LITTLE_FOOT_PRINT)
  uint32_t xferLen;
  uint8_t *b;
  union
  {
    uint16_t val;
    struct
    {
      uint8_t lsb;
      uint8_t msb;
    };
  } t;
  while (len)
  {
    xferLen = (len < SPI_MAX_PIXELS_AT_ONCE) ? len : SPI_MAX_PIXELS_AT_ONCE;
    b = _buffer.v8;
    for (uint32_t i = 0; i < xferLen; i++)
    {
      t.val = *data++;
      *b++ = t.msb;
      *b++ = t.lsb;
    }
    len -= xferLen;

    xferLen += xferLen; // uint16_t to uint8_t, double length
    WRITEBUF(_buffer.v8, xferLen);
  }
#endif // !defined(LITTLE_FOOT_PRINT)
}

#if !defined(LITTLE_FOOT_PRINT)
void Arduino_HWSPI::writeBytes(uint8_t *data, uint32_t len)
{
  WRITEBUF(data, len);
}

void Arduino_HWSPI::writePattern(uint8_t *data, uint8_t len, uint32_t repeat)
{
#if defined(ESP8266) || defined(ESP32)
  HWSPI.writePattern(data, len, repeat);
#else  // !(defined(ESP8266) || defined(ESP32))
  while (repeat--)
  {
    WRITEBUF(data, len);
  }
#endif // !(defined(ESP8266) || defined(ESP32))
}
#endif // !defined(LITTLE_FOOT_PRINT)

INLINE void Arduino_HWSPI::WRITE(uint8_t d)
{
#if defined(SPI_HAS_TRANSACTION)
  HWSPI.transfer(d);
#elif defined(__AVR__) || defined(CORE_TEENSY)
  SPCRbackup = SPCR;
  SPCR = mySPCR;
  HWSPI.transfer(d);
  SPCR = SPCRbackup;
#elif defined(__arm__)
  HWSPI.setClockDivider(21); //4MHz
  HWSPI.setDataMode(_dataMode);
  HWSPI.transfer(d);
#endif
}

#if !defined(LITTLE_FOOT_PRINT)
INLINE void Arduino_HWSPI::WRITE16(uint16_t d)
{
#if defined(ESP8266) || defined(ESP32)
  HWSPI.write16(d);
#elif defined(SPI_HAS_TRANSACTION)
  HWSPI.transfer16(d);
#elif defined(__AVR__) || defined(CORE_TEENSY)
  SPCRbackup = SPCR;
  SPCR = mySPCR;
  HWSPI.transfer16(d);
  SPCR = SPCRbackup;
#elif defined(__arm__)
  HWSPI.setClockDivider(21); //4MHz
  HWSPI.setDataMode(_dataMode);
  HWSPI.transfer16(d);
#else
  HWSPI.transfer16(d);
#endif
}

INLINE void Arduino_HWSPI::WRITEBUF(uint8_t *buf, size_t count)
{
#if defined(ESP8266) || defined(ESP32)
  HWSPI.writeBytes(buf, count);
#else  // !(defined(ESP8266) || defined(ESP32))
  HWSPI.transfer(buf, count);
#endif // !(defined(ESP8266) || defined(ESP32))
}
#endif // !defined(LITTLE_FOOT_PRINT)

/******** low level bit twiddling **********/

INLINE void Arduino_HWSPI::DC_HIGH(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_dcPortSet = 1;
#else  // !KINETISK
  *_dcPortSet = _dcPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
  *_dcPort |= _dcPinMaskSet;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_dc, HIGH);
#endif // end !USE_FAST_PINIO
}

INLINE void Arduino_HWSPI::DC_LOW(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_dcPortClr = 1;
#else  // !KINETISK
  *_dcPortClr = _dcPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
  *_dcPort &= _dcPinMaskClr;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_dc, LOW);
#endif // end !USE_FAST_PINIO
}

INLINE void Arduino_HWSPI::CS_HIGH(void)
{
  if (_cs >= 0)
  {
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
    *_csPortSet = 1;
#else  // !KINETISK
    *_csPortSet = _csPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
    *_csPort |= _csPinMaskSet;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
    digitalWrite(_cs, HIGH);
#endif // end !USE_FAST_PINIO
  }
}

INLINE void Arduino_HWSPI::CS_LOW(void)
{
  if (_cs >= 0)
  {
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
    *_csPortClr = 1;
#else  // !KINETISK
    *_csPortClr = _csPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
    *_csPort &= _csPinMaskClr;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
    digitalWrite(_cs, LOW);
#endif // end !USE_FAST_PINIO
  }
}
