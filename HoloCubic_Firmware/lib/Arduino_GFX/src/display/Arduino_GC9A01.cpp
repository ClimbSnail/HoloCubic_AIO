#include "Arduino_GC9A01.h"
#include "SPI.h"

Arduino_GC9A01::Arduino_GC9A01(
    Arduino_DataBus *bus, int8_t rst, uint8_t r,
    bool ips, int16_t w, int16_t h,
    uint8_t col_offset1, uint8_t row_offset1, uint8_t col_offset2, uint8_t row_offset2)
    : Arduino_TFT(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2)
{
}

void Arduino_GC9A01::begin(int32_t speed)
{
  _override_datamode = SPI_MODE0; // always use SPI_MODE0
  Arduino_TFT::begin(speed);
}

void Arduino_GC9A01::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW) || (y != _currentY) || (h != _currentH))
  {
    _bus->writeC8D16D16(GC9A01_CASET, x + _xStart, x + w - 1 + _xStart);
    _bus->writeC8D16D16(GC9A01_RASET, y + _yStart, y + h - 1 + _yStart);

    _currentX = x;
    _currentY = y;
    _currentW = w;
    _currentH = h;
  }

  _bus->writeCommand(GC9A01_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_GC9A01::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 3:
    r = 0xA8;
    break;
  case 2:
    r = 0xC8;
    break;
  case 1:
    r = 0x68;
    break;
  default: // case 0:
    r = 0x08;
    break;
  }

  _bus->beginWrite();
  _bus->writeCommand(GC9A01_MADCTL);
  _bus->write(r);
  _bus->endWrite();
}

void Arduino_GC9A01::invertDisplay(bool i)
{
  _bus->sendCommand(_ips ? (i ? GC9A01_INVOFF : GC9A01_INVON) : (i ? GC9A01_INVON : GC9A01_INVOFF));
}

void Arduino_GC9A01::displayOn(void)
{
  _bus->sendCommand(GC9A01_SLPOUT);
  delay(GC9A01_SLPOUT_DELAY);
}

void Arduino_GC9A01::displayOff(void)
{
  _bus->sendCommand(GC9A01_SLPIN);
  delay(GC9A01_SLPIN_DELAY);
}

void Arduino_GC9A01::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(GC9A01_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(GC9A01_RST_DELAY);
  }
  else
  {
    // Software Rest
  }

  uint8_t gc9a01_init_operations[] = {
      BEGIN_WRITE,

      WRITE_COMMAND_8, 0xEF,
      WRITE_C8_D8, 0xEB, 0x14,

      WRITE_COMMAND_8, 0xFE,
      WRITE_COMMAND_8, 0xEF,

      WRITE_C8_D8, 0xEB, 0x14,

      WRITE_C8_D8, 0x84, 0x40,

      WRITE_C8_D8, 0x85, 0xFF,

      WRITE_C8_D8, 0x86, 0xFF,

      WRITE_C8_D8, 0x87, 0xFF,

      WRITE_C8_D8, 0x88, 0x0A,

      WRITE_C8_D8, 0x89, 0x21,

      WRITE_C8_D8, 0x8A, 0x00,

      WRITE_C8_D8, 0x8B, 0x80,

      WRITE_C8_D8, 0x8C, 0x01,

      WRITE_C8_D8, 0x8D, 0x01,

      WRITE_C8_D8, 0x8E, 0xFF,

      WRITE_C8_D8, 0x8F, 0xFF,

      WRITE_C8_D16, 0xB6, 0x00, 0x20,

      WRITE_C8_D8, 0x3A, 0x05,

      WRITE_COMMAND_8, 0x90,
      WRITE_BYTES, 4, 0x08, 0x08, 0x08, 0x08,

      WRITE_C8_D8, 0xBD, 0x06,

      WRITE_C8_D8, 0xBC, 0x00,

      WRITE_COMMAND_8, 0xFF,
      WRITE_BYTES, 3, 0x60, 0x01, 0x04,

      WRITE_C8_D8, 0xC3, 0x13,
      WRITE_C8_D8, 0xC4, 0x13,

      WRITE_C8_D8, 0xC9, 0x22,

      WRITE_C8_D8, 0xBE, 0x11,

      WRITE_C8_D16, 0xE1, 0x10, 0x0E,

      WRITE_COMMAND_8, 0xDF,
      WRITE_BYTES, 3, 0x21, 0x0c, 0x02,

      WRITE_COMMAND_8, 0xF0,
      WRITE_BYTES, 6,
      0x45, 0x09, 0x08, 0x08, 0x26,
      0x2A,

      WRITE_COMMAND_8, 0xF1,
      WRITE_BYTES, 6,
      0x43, 0x70, 0x72, 0x36, 0x37,
      0x6F,

      WRITE_COMMAND_8, 0xF2,
      WRITE_BYTES, 6,
      0x45, 0x09, 0x08, 0x08, 0x26,
      0x2A,

      WRITE_COMMAND_8, 0xF3,
      WRITE_BYTES, 6,
      0x43, 0x70, 0x72, 0x36, 0x37,
      0x6F,

      WRITE_C8_D16, 0xED, 0x1B, 0x0B,

      WRITE_C8_D8, 0xAE, 0x77,

      WRITE_C8_D8, 0xCD, 0x63,

      WRITE_COMMAND_8, 0x70,
      WRITE_BYTES, 9,
      0x07, 0x07, 0x04, 0x0E, 0x0F,
      0x09, 0x07, 0x08, 0x03,

      WRITE_C8_D8, 0xE8, 0x34,

      WRITE_COMMAND_8, 0x62,
      WRITE_BYTES, 12,
      0x18, 0x0D, 0x71, 0xED, 0x70,
      0x70, 0x18, 0x0F, 0x71, 0xEF,
      0x70, 0x70,

      WRITE_COMMAND_8, 0x63,
      WRITE_BYTES, 12,
      0x18, 0x11, 0x71, 0xF1, 0x70,
      0x70, 0x18, 0x13, 0x71, 0xF3,
      0x70, 0x70,

      WRITE_COMMAND_8, 0x64,
      WRITE_BYTES, 7,
      0x28, 0x29, 0xF1, 0x01, 0xF1,
      0x00, 0x07,

      WRITE_COMMAND_8, 0x66,
      WRITE_BYTES, 10,
      0x3C, 0x00, 0xCD, 0x67, 0x45,
      0x45, 0x10, 0x00, 0x00, 0x00,

      WRITE_COMMAND_8, 0x67,
      WRITE_BYTES, 10,
      0x00, 0x3C, 0x00, 0x00, 0x00,
      0x01, 0x54, 0x10, 0x32, 0x98,

      WRITE_COMMAND_8, 0x74,
      WRITE_BYTES, 7,
      0x10, 0x85, 0x80, 0x00, 0x00,
      0x4E, 0x00,

      WRITE_C8_D16, 0x98, 0x3e, 0x07,

      WRITE_COMMAND_8, 0x35,

      WRITE_COMMAND_8, GC9A01_SLPOUT,
      END_WRITE,

      DELAY, GC9A01_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, GC9A01_DISPON, // Display on
      END_WRITE,

      DELAY, 20};

  _bus->batchOperation(gc9a01_init_operations, sizeof(gc9a01_init_operations));

  if (_ips)
  {
    _bus->sendCommand(GC9A01_INVON);
  }
}
