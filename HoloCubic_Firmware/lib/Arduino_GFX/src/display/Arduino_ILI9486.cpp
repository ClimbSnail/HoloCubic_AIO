/*
 * start rewrite from:
 * https://github.com/nopnop2002/esp-idf-parallel-tft
 */
#include "Arduino_ILI9486.h"

Arduino_ILI9486::Arduino_ILI9486(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT(bus, rst, r, ips, ILI9486_TFTWIDTH, ILI9486_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_ILI9486::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ILI9486::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 1:
    r = (ILI9486_MADCTL_MY | ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
    break;
  case 2:
    r = (ILI9486_MADCTL_BGR);
    break;
  case 3:
    r = (ILI9486_MADCTL_MX | ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
    break;
  default: // case 0:
    r = (ILI9486_MADCTL_MX | ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR);
    break;
  }
  _bus->beginWrite();
  _bus->writeC8D8(ILI9486_MADCTL, r);
  _bus->endWrite();
}

void Arduino_ILI9486::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _bus->writeC8D16D16(ILI9486_CASET, x, x + w - 1);
  }
  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    _bus->writeC8D16D16(ILI9486_PASET, y, y + h - 1);
  }

  _bus->writeCommand(ILI9486_RAMWR); // write to RAM
}

void Arduino_ILI9486::invertDisplay(bool i)
{
  _bus->sendCommand((_ips ^ i) ? ILI9486_INVON : ILI9486_INVOFF);
}

void Arduino_ILI9486::displayOn(void)
{
  _bus->sendCommand(ILI9486_SLPOUT);
  delay(ILI9486_SLPOUT_DELAY);
}

void Arduino_ILI9486::displayOff(void)
{
  _bus->sendCommand(ILI9486_SLPIN);
  delay(ILI9486_SLPIN_DELAY);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ILI9486::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ILI9486_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ILI9486_RST_DELAY);
  }

  uint8_t ili9486_init_operations[] = {
      BEGIN_WRITE,
      WRITE_COMMAND_8, ILI9486_SWRESET,
      END_WRITE,

      DELAY, ILI9486_RST_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, ILI9486_DISPOFF,
      WRITE_C8_D8, ILI9486_PIXFMT, 0x55, // 16-bit color
      WRITE_C8_D8, 0xB0, 0x00,           // unlocks E0, F0
      WRITE_COMMAND_8, 0xB3,
      WRITE_BYTES, 4, 0x02, 0x00, 0x00, 0x00, //Frame Memory, interface [02 00 00 00]
      WRITE_C8_D8, 0xB4, 0x00,                // Frame mode [00]
      // WRITE_COMMAND_8, 0xB6,
      // WRITE_BYTES, 3, 0x02, 0x02, 0x3B,  // Display Function Control [02 02 3B]
      WRITE_COMMAND_8, 0xB6,
      WRITE_BYTES, 3, 0x02, 0x42, 0x3B, // Display Function Control [02 02 3B]
      WRITE_COMMAND_8, 0xD0,
      WRITE_BYTES, 3, 0x07, 0x42, 0x18,
      WRITE_COMMAND_8, 0xD1,
      WRITE_BYTES, 3, 0x00, 0x07, 0x18,
      WRITE_C8_D16, 0xD2, 0x01, 0x02,
      WRITE_C8_D16, 0xD3, 0x01, 0x02, // Set Power for Partial Mode [01 22]
      WRITE_C8_D16, 0xD4, 0x01, 0x02, // Set Power for Idle Mode [01 22]
      // WRITE_COMMAND_8, 0xC0,
      // 5, 0x10, 0x3B, 0x00, 0x02, 0x11,
      WRITE_COMMAND_8, 0xC0,
      WRITE_BYTES, 5, 0x14, 0x3B, 0x00, 0x02, 0x11,
      WRITE_COMMAND_8, 0xC1,
      WRITE_BYTES, 3, 0x10, 0x10, 0x88, // Display Timing Normal [10 10 88]
      WRITE_C8_D8, 0xC5, 0x03,          //Frame Rate [03]
      WRITE_C8_D8, 0xC6, 0x02,          //Interface Control [02]

      WRITE_COMMAND_8, 0xC8,
      WRITE_BYTES, 12,
      0x00, 0x32, 0x36, 0x45, 0x06,
      0x16, 0x37, 0x75, 0x77, 0x54,
      0x0C, 0x00,

      WRITE_C8_D8, 0xCC, 0x00, // Panel Control [00]
      WRITE_C8_D8, 0x36, 0x18, // 0x08,

      WRITE_COMMAND_8, ILI9486_GMCTRP1,
      WRITE_BYTES, 15,
      0x0F, 0x1F, 0x1C, 0x0C, 0x0F,
      0x08, 0x48, 0x98, 0x37, 0x0A,
      0x13, 0x04, 0x11, 0x0D, 0x00,

      WRITE_COMMAND_8, ILI9486_GMCTRN1,
      WRITE_BYTES, 15,
      0x0F, 0x32, 0x2E, 0x0B, 0x0D,
      0x05, 0x47, 0x75, 0x37, 0x06,
      0x10, 0x03, 0x24, 0x20, 0x00,

      WRITE_COMMAND_8, ILI9486_SLPOUT, // Exit Sleep
      END_WRITE,

      DELAY, ILI9486_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, ILI9486_DISPON, // Display on
      END_WRITE};

  _bus->batchOperation(ili9486_init_operations, sizeof(ili9486_init_operations));

  if (_ips)
  {
    _bus->sendCommand(ILI9486_INVON);
  }
  else
  {
    _bus->sendCommand(ILI9486_INVOFF);
  }
}
