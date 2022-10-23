#include "Arduino_GC9106.h"
#include "SPI.h"

Arduino_GC9106::Arduino_GC9106(
    Arduino_DataBus *bus, int8_t rst, uint8_t r,
    bool ips, int16_t w, int16_t h,
    uint8_t col_offset1, uint8_t row_offset1, uint8_t col_offset2, uint8_t row_offset2)
    : Arduino_TFT(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2)
{
}

void Arduino_GC9106::begin(int32_t speed)
{
  _override_datamode = SPI_MODE0; // always use SPI_MODE0
  Arduino_TFT::begin(speed);
}

void Arduino_GC9106::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW) || (y != _currentY) || (h != _currentH))
  {
    _bus->writeC8D16D16(GC9106_CASET, x + _xStart, x + w - 1 + _xStart);
    _bus->writeC8D16D16(GC9106_RASET, y + _yStart, y + h - 1 + _yStart);

    _currentX = x;
    _currentY = y;
    _currentW = w;
    _currentH = h;
  }

  _bus->writeCommand(GC9106_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_GC9106::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 3:
    r = GC9106_MADCTL_MY | GC9106_MADCTL_MV | GC9106_MADCTL_BGR;
    break;
  case 2:
    r = GC9106_MADCTL_MY | GC9106_MADCTL_MX | GC9106_MADCTL_BGR;
    break;
  case 1:
    r = GC9106_MADCTL_MX | GC9106_MADCTL_MV | GC9106_MADCTL_BGR;
    break;
  default: // case 0:
    r = GC9106_MADCTL_BGR;
    break;
  }
  _bus->beginWrite();
  _bus->writeCommand(GC9106_MADCTL);
  _bus->write(r);
  _bus->endWrite();
}

void Arduino_GC9106::invertDisplay(bool i)
{
  _bus->sendCommand(_ips ? (i ? GC9106_INVOFF : GC9106_INVON) : (i ? GC9106_INVON : GC9106_INVOFF));
}

void Arduino_GC9106::displayOn(void)
{
  _bus->sendCommand(GC9106_SLPOUT);
  delay(GC9106_SLPOUT_DELAY);
}

void Arduino_GC9106::displayOff(void)
{
  _bus->sendCommand(GC9106_SLPIN);
  delay(GC9106_SLPIN_DELAY);
}

void Arduino_GC9106::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(GC9106_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(GC9106_RST_DELAY);
  }
  else
  {
    // Software Rest
  }

  uint8_t GC9106_init_operations[] = {
      BEGIN_WRITE,

      WRITE_COMMAND_8, GC9106_INTERRE1,
      WRITE_COMMAND_8, GC9106_INTERRE1,
      WRITE_COMMAND_8, GC9106_INTERRE1,
      WRITE_COMMAND_8, GC9106_INTERRE2,

      WRITE_C8_D8, GC9106_FRMCTR3, 0x03,

      WRITE_C8_D8, GC9106_MADCTL, 0xD8,

      WRITE_C8_D8, GC9106_COLMOD, 0x05,
      
      WRITE_C8_D8, 0xB6, 0x11,
      WRITE_C8_D8, 0xAC, 0x0B,

      WRITE_C8_D8, GC9106_INVCTR, 0x21,

      WRITE_C8_D8, GC9106_FRMCTR1, 0xC0,

      WRITE_C8_D16, GC9106_VREG1CTL, 0x50, 0x43,
      WRITE_C8_D16, GC9106_VREG2CTL, 0x56, 0x43,

      WRITE_COMMAND_8, GC9106_GAMMA1,
      WRITE_BYTES, 14, 0x1F, 0x41, 0x1B, 0x55, 0x36, 0x3D, 0x3E,
                       0x00, 0x16, 0x08, 0x09, 0x15, 0x14, 0x0F,

      WRITE_COMMAND_8, GC9106_GAMMA2,
      WRITE_BYTES, 14, 0x1F, 0x41, 0x1B, 0x55, 0x36, 0x3D, 0x3E,
                       0x00, 0x16, 0x08, 0x09, 0x15, 0x14, 0x0F,

      WRITE_COMMAND_8, GC9106_INTERRE1,
      WRITE_COMMAND_8, 0xFF,

      WRITE_C8_D8, GC9106_TELON, 0x00,
      WRITE_C8_D8, GC9106_SCANLSET, 0x00,

      WRITE_COMMAND_8, GC9106_SLPOUT,
      END_WRITE,

      DELAY, GC9106_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, GC9106_DISPON, // Display on
      END_WRITE,

      DELAY, 20
      };

  _bus->batchOperation(GC9106_init_operations, sizeof(GC9106_init_operations));

  if (_ips)
  {
    _bus->sendCommand(GC9106_INVON);
  }
}
