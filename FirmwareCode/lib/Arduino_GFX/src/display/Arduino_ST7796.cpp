/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_ST7796.h"

Arduino_ST7796::Arduino_ST7796(
    Arduino_DataBus *bus, int8_t rst, uint8_t r,
    bool ips, int16_t w, int16_t h,
    uint8_t col_offset1, uint8_t row_offset1, uint8_t col_offset2, uint8_t row_offset2)
    : Arduino_TFT(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2)
{
}

void Arduino_ST7796::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ST7796::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 0:
    // r = ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_BGR | ST7796_MADCTL_MH;
    r = ST7796_MADCTL_MY | ST7796_MADCTL_BGR;
    break;

  case 1:
    // r = ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR | ST7796_MADCTL_MH;
    r = ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR;
    break;

  case 2:
    r = ST7796_MADCTL_MX | ST7796_MADCTL_ML | ST7796_MADCTL_BGR | ST7796_MADCTL_MH;
    r = ST7796_MADCTL_MX | ST7796_MADCTL_BGR;
    break;

  case 3:
    // r = ST7796_MADCTL_MX | ST7796_MADCTL_MV | ST7796_MADCTL_BGR | ST7796_MADCTL_MH;
    r = ST7796_MADCTL_MV | ST7796_MADCTL_BGR;
    break;
  }

  _bus->beginWrite();
  _bus->writeCommand(ST7796_MADCTL);
  _bus->write(r);
  _bus->endWrite();
}

void Arduino_ST7796::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _bus->writeC8D16D16(ST7796_CASET, x, x + w - 1);
  }

  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    x += _yStart;
    _bus->writeC8D16D16(ST7796_RASET, y, y + h - 1);
  }

  _bus->writeCommand(ST7796_RAMWR); // write to RAM
}

void Arduino_ST7796::invertDisplay(bool i)
{
  _bus->sendCommand(_ips ? (i ? ST7796_INVOFF : ST7796_INVON) : (i ? ST7796_INVON : ST7796_INVOFF));
}

void Arduino_ST7796::displayOn(void)
{
  _bus->sendCommand(ST7796_SLPOUT);
  delay(ST7796_SLPOUT_DELAY);
}

void Arduino_ST7796::displayOff(void)
{
  _bus->sendCommand(ST7796_SLPIN);
  delay(ST7796_SLPIN_DELAY);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ST7796::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ST7796_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ST7796_RST_DELAY);
  }
  else
  {
    // Software Rest
    _bus->sendCommand(ST7796_SWRESET);
    delay(ST7796_RST_DELAY);
  }

  _bus->beginWrite();
  _bus->writeC8D8(0xF0, 0xC3);
  _bus->writeC8D8(0xF0, 0x96);
  _bus->writeC8D8(0x3A, 0x05);
  _bus->writeC8D8(0xB0, 0x80);
  _bus->writeC8D16(0xB6, 0x0002);
  _bus->writeC8D16D16(0xB5, 0x0203, 0x0004);
  _bus->writeC8D16(0xB1, 0x8010);
  _bus->writeC8D8(0xB4, 0x00);
  _bus->writeC8D8(0xB7, 0xC6);
  _bus->writeC8D8(0xC5, 0x24);
  _bus->writeC8D8(0xE4, 0x31);
  _bus->writeCommand(0xE8);
  _bus->write16(0x408A);
  _bus->write16(0x0000);
  _bus->write16(0x2919);
  _bus->write16(0xA533);
  _bus->writeCommand(0xC2);
  _bus->writeCommand(0xA7);

  _bus->writeCommand(0xE0);
  _bus->write16(0xF009);
  _bus->write16(0x1312);
  _bus->write16(0x122B);
  _bus->write16(0x3C44);
  _bus->write16(0x4B1B);
  _bus->write16(0x1817);
  _bus->write16(0x1D21);

  _bus->writeCommand(0XE1);
  _bus->write16(0xF009);
  _bus->write16(0x130C);
  _bus->write16(0x0D27);
  _bus->write16(0x3B44);
  _bus->write16(0x4D0B);
  _bus->write16(0x1717);
  _bus->write16(0x1D21);

  _bus->writeC8D8(0xF0, 0xC3);
  _bus->writeC8D16(0xF0, 0x69);
  _bus->writeCommand(0X13);
  _bus->writeCommand(0X11);
  if (_ips)
  {
    _bus->writeCommand(ST7796_INVON);
  }
  _bus->writeCommand(0X29);
  _bus->endWrite();
}
