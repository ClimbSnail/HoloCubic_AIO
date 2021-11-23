/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_HX8347D.h"
#include "SPI.h"

Arduino_HX8347D::Arduino_HX8347D(
    Arduino_DataBus *bus, int8_t rst, uint8_t r,
    bool ips, int16_t w, int16_t h,
    uint8_t col_offset1, uint8_t row_offset1, uint8_t col_offset2, uint8_t row_offset2)
    : Arduino_TFT(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2)
{
}

void Arduino_HX8347D::begin(int32_t speed)
{
#if defined(__AVR__)
  _override_datamode = SPI_MODE0;
#endif
  Arduino_TFT::begin(speed);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_HX8347D::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(HX8347D_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(HX8347D_RST_DELAY);
  }
  else
  {
    // Software Rest
  }

  uint8_t hx8347d_init_operations[] = {
      BEGIN_WRITE,
      WRITE_C8_D8, 0xEA, 0x00,
      WRITE_C8_D8, 0xEB, 0x20,
      WRITE_C8_D8, 0xEC, 0x0C,
      WRITE_C8_D8, 0xED, 0xC4,
      WRITE_C8_D8, 0xE8, 0x38,
      WRITE_C8_D8, 0xE9, 0x10,
      WRITE_C8_D8, 0xF1, 0x01,
      WRITE_C8_D8, 0xF2, 0x10,
      WRITE_C8_D8, 0x40, 0x01,
      WRITE_C8_D8, 0x41, 0x00,
      WRITE_C8_D8, 0x42, 0x00,
      WRITE_C8_D8, 0x43, 0x10,
      WRITE_C8_D8, 0x44, 0x0E,
      WRITE_C8_D8, 0x45, 0x24,
      WRITE_C8_D8, 0x46, 0x04,
      WRITE_C8_D8, 0x47, 0x50,
      WRITE_C8_D8, 0x48, 0x02,
      WRITE_C8_D8, 0x49, 0x13,
      WRITE_C8_D8, 0x4A, 0x19,
      WRITE_C8_D8, 0x4B, 0x19,
      WRITE_C8_D8, 0x4C, 0x16,
      WRITE_C8_D8, 0x50, 0x1B,
      WRITE_C8_D8, 0x51, 0x31,
      WRITE_C8_D8, 0x52, 0x2F,
      WRITE_C8_D8, 0x53, 0x3F,
      WRITE_C8_D8, 0x54, 0x3F,
      WRITE_C8_D8, 0x55, 0x3E,
      WRITE_C8_D8, 0x56, 0x2F,
      WRITE_C8_D8, 0x57, 0x7B,
      WRITE_C8_D8, 0x58, 0x09,
      WRITE_C8_D8, 0x59, 0x06,
      WRITE_C8_D8, 0x5A, 0x06,
      WRITE_C8_D8, 0x5B, 0x0C,
      WRITE_C8_D8, 0x5C, 0x1D,
      WRITE_C8_D8, 0x5D, 0xCC,
      WRITE_C8_D8, 0x1B, 0x1B,
      WRITE_C8_D8, 0x1A, 0x01,
      WRITE_C8_D8, 0x24, 0x2F,
      WRITE_C8_D8, 0x25, 0x57,
      WRITE_C8_D8, 0x23, 0x88,
      WRITE_C8_D8, 0x18, 0x34,
      WRITE_C8_D8, 0x19, 0x01,
      WRITE_C8_D8, 0x1F, 0x88,
      WRITE_C8_D8, 0x1F, 0x80,
      WRITE_C8_D8, 0x1F, 0x90,
      WRITE_C8_D8, 0x1F, 0xD0,
      WRITE_C8_D8, 0x17, 0x05,
      WRITE_C8_D8, 0x28, 0x38,
      WRITE_C8_D8, 0x28, 0x3F,
      WRITE_C8_D8, 0x16, 0x18,
      END_WRITE};

  _bus->batchOperation(hx8347d_init_operations, sizeof(hx8347d_init_operations));

  if (_ips)
  {
    _bus->beginWrite();
    _bus->writeC8D8(0x01, 0x02);
    _bus->endWrite();
  }
}

void Arduino_HX8347D::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    int16_t x_start = x + _xStart, x_end = x + w - 1 + _xStart;
    _bus->writeC8D8(0x02, x_start >> 8);
    _bus->writeC8D8(0x03, x_start & 0xFF);
    _bus->writeC8D8(0x04, x_end >> 8);
    _bus->writeC8D8(0x05, x_end & 0xFF);

    _currentX = x;
    _currentW = w;
  }
  if ((y != _currentY) || (h != _currentH))
  {
    int16_t y_start = y + _yStart, y_end = y + h - 1 + _yStart;
    _bus->writeC8D8(0x06, y_start >> 8);
    _bus->writeC8D8(0x07, y_start & 0xFF);
    _bus->writeC8D8(0x08, y_end >> 8);
    _bus->writeC8D8(0x09, y_end & 0xFF);

    _currentY = y;
    _currentH = h;
  }

  _bus->writeCommand(0x22); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_HX8347D::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  _bus->beginWrite();
  switch (_rotation)
  {
  case 0:
    _bus->writeC8D8(0x36, 0x07);
    _bus->writeC8D8(0x16, 0x40);
    break;
  case 1:
    _bus->writeC8D8(0x36, 0x07);
    _bus->writeC8D8(0x16, 0x20);
    break;
  case 2:
    _bus->writeC8D8(0x36, 0x03);
    _bus->writeC8D8(0x16, 0x00);
    break;
  case 3:
    _bus->writeC8D8(0x36, 0x03);
    _bus->writeC8D8(0x16, 0x60);
    break;
  }
  _bus->endWrite();
}

void Arduino_HX8347D::invertDisplay(bool i)
{
  _bus->beginWrite();
  if (_ips && i)
  {
    _bus->writeC8D8(0x01, 0x00);
  }
  else
  {
    _bus->writeC8D8(0x01, 0x02);
  }
  _bus->endWrite();
}

void Arduino_HX8347D::displayOn(void)
{
  _bus->sendCommand(0x28);
  _bus->sendData(0x3C); //GON=1, DTE=1, D=11
}

void Arduino_HX8347D::displayOff(void)
{
  _bus->sendCommand(0x28);
  _bus->sendData(0x34); //GON=1, DTE=1, D=01
  delay(40);
}
