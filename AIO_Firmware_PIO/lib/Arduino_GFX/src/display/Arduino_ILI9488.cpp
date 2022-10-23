/*
 * start rewrite from:
 * https://github.com/nopnop2002/esp-idf-parallel-tft
 */
#include "Arduino_ILI9488.h"

Arduino_ILI9488::Arduino_ILI9488(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT(bus, rst, r, ips, ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_ILI9488::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ILI9488::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 1:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MV | ILI9488_MADCTL_MX | ILI9488_MADCTL_MY);
    break;
  case 2:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MX);
    break;
  case 3:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MV);
    break;
  default: // case 0:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MY);
    break;
  }
  _bus->beginWrite();
  _bus->writeC8D8(ILI9488_MADCTL, r);
  _bus->endWrite();
}

void Arduino_ILI9488::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _bus->writeC8D16D16(ILI9488_CASET, x, x + w - 1);
  }
  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    _bus->writeC8D16D16(ILI9488_PASET, y, y + h - 1);
  }

  _bus->writeCommand(ILI9488_RAMWR); // write to RAM
}

void Arduino_ILI9488::invertDisplay(bool i)
{
  _bus->sendCommand(i ? ILI9488_INVON : ILI9488_INVOFF);
}

void Arduino_ILI9488::displayOn(void)
{
  _bus->sendCommand(ILI9488_SLPOUT);
  delay(ILI9488_SLPOUT_DELAY);
}

void Arduino_ILI9488::displayOff(void)
{
  _bus->sendCommand(ILI9488_SLPIN);
  delay(ILI9488_SLPIN_DELAY);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ILI9488::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ILI9488_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ILI9488_RST_DELAY);
  }

  uint8_t ili9488_init_operations[] = {
      BEGIN_WRITE,
      WRITE_COMMAND_8, ILI9488_SWRESET,
      END_WRITE,

      DELAY, ILI9488_RST_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, 0x28,   //Display Off
      WRITE_C8_D8, 0x3A, 0x55, //Pixel read=565, write=565.

      WRITE_C8_D16, 0xC0, 0x10, 0x10, //Power Control 1 [0E 0E]
      WRITE_C8_D8, 0xC1, 0x41,        //Power Control 2 [43]
      WRITE_COMMAND_8, 0xC5,
      WRITE_BYTES, 4, 0x00, 0x22, 0x80, 0x40, //VCOM  Control 1 [00 40 00 40]
      WRITE_C8_D8, 0x36, 0x98,                //Memory Access [00]
      WRITE_C8_D8, 0xB0, 0x00,                //Interface     [00]
      WRITE_C8_D16, 0xB1, 0xB0, 0x11,         //Frame Rate Control [B0 11]
      WRITE_C8_D8, 0xB4, 0x02,                //Inversion Control [02]
      WRITE_COMMAND_8, 0xB6,
      WRITE_BYTES, 3, 0x02, 0x02, 0x3B, // Display Function Control [02 02 3B] .kbv NL=480
      WRITE_C8_D8, 0xB7, 0xC6,          //Entry Mode      [06]
      WRITE_C8_D8, 0x3A, 0x55,          //Interlace Pixel Format [XX]
      WRITE_COMMAND_8, 0xF7,
      WRITE_BYTES, 4, 0xA9, 0x51, 0x2C, 0x82, //Adjustment Control 3 [A9 51 2C 82]
      WRITE_COMMAND_8, ILI9488_SLPOUT,        // Sleep Out
      END_WRITE,

      DELAY, ILI9488_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, ILI9488_DISPON, // Display on
      END_WRITE};

  _bus->batchOperation(ili9488_init_operations, sizeof(ili9488_init_operations));

  if (_ips)
  {
    _bus->sendCommand(ILI9488_INVON);
  }
  else
  {
    _bus->sendCommand(ILI9488_INVOFF);
  }
}
