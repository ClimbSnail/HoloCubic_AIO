/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_ILI9488_18bit.h"

Arduino_ILI9488_18bit::Arduino_ILI9488_18bit(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT_18bit(bus, rst, r, ips, ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_ILI9488_18bit::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ILI9488_18bit::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ILI9488_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ILI9488_RST_DELAY);
  }
  else
  {
    // Software Rest
    _bus->sendCommand(ILI9488_SWRST);
    delay(ILI9488_RST_DELAY);
  }

  uint8_t ili9488_init_operations[] = {
      BEGIN_WRITE,

      WRITE_COMMAND_8, 0xE0,
      WRITE_BYTES, 15,
      0x00, 0x03, 0x09, 0x08,
      0x16, 0x0A, 0x3F, 0x78,
      0x4C, 0x09, 0x0A, 0x08,
      0x16, 0x1A, 0x0F,

      WRITE_COMMAND_8, 0XE1,
      WRITE_BYTES, 15,
      0x00, 0x16, 0x19, 0x03,
      0x0F, 0x05, 0x32, 0x45,
      0x46, 0x04, 0x0E, 0x0D,
      0x35, 0x37, 0x0F,

      WRITE_C8_D16, 0XC0, // Power Control 1
      0x17,               // Vreg1out
      0x15,               // Verg2out

      WRITE_C8_D8, 0xC1, // Power Control 2
      0x41,              // VGH,VGL

      WRITE_COMMAND_8, 0xC5, // Power Control 3
      WRITE_BYTES, 3,
      0x00,
      0x12, // Vcom
      0x80,

      WRITE_C8_D8, 0x36, 0x48, // Memory Access

      WRITE_C8_D8, 0x3A, 0x66, // Interface Pixel Format, 18 bit

      WRITE_C8_D8, 0xB0, 0x80, // Interface Mode Control, SDO NOT USE

      WRITE_C8_D8, 0xB1, 0xA0, // Frame rate, 60Hz

      WRITE_C8_D8, 0xB4, 0x02, // Display Inversion Control, 2-dot

      WRITE_C8_D16, 0XB6, // Display Function Control  RGB/MCU Interface Control
      0x02,               // MCU
      0x02,               // Source,Gate scan dieection

      WRITE_C8_D8, 0XE9, 0x00, // Set Image Function, Disable 24 bit data

      WRITE_COMMAND_8, 0xF7,                  // Adjust Control
      WRITE_BYTES, 4, 0xA9, 0x51, 0x2C, 0x82, // D7 stream, loose

      WRITE_COMMAND_8, ILI9488_SLPOUT, // Exit Sleep
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

void Arduino_ILI9488_18bit::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    int16_t x_start = x + _xStart, x_end = x + w - 1 + _xStart;

    _bus->writeCommand(ILI9488_CASET); // Column addr set
    _bus->write(x_start >> 8);
    _bus->write(x_start & 0xFF); // XSTART
    _bus->write(x_end >> 8);
    _bus->write(x_end & 0xFF); // XEND

    _currentX = x;
    _currentW = w;
  }
  if ((y != _currentY) || (h != _currentH))
  {
    int16_t y_start = y + _yStart, y_end = y + h - 1 + _yStart;

    _bus->writeCommand(ILI9488_PASET); // Row addr set
    _bus->write(y_start >> 8);
    _bus->write(y_start & 0xFF); // YSTART
    _bus->write(y_end >> 8);
    _bus->write(y_end & 0xFF); // YEND

    _currentY = y;
    _currentH = h;
  }

  _bus->writeCommand(ILI9488_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ILI9488_18bit::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 0:
    r = (ILI9488_MAD_BGR | ILI9488_MAD_MY);
    break;
  case 1:
    r = (ILI9488_MAD_BGR | ILI9488_MAD_MV | ILI9488_MAD_MX | ILI9488_MAD_MY);
    break;
  case 2:
    r = (ILI9488_MAD_BGR | ILI9488_MAD_MX);
    break;
  case 3:
    r = (ILI9488_MAD_BGR | ILI9488_MAD_MV);
    break;
  }

  _bus->beginWrite();
  _bus->writeC8D8(ILI9488_MADCTL, r);
  _bus->endWrite();
}

void Arduino_ILI9488_18bit::invertDisplay(bool i)
{
  _bus->sendCommand(i ? ILI9488_INVON : ILI9488_INVOFF);
}

void Arduino_ILI9488_18bit::displayOn(void)
{
  _bus->sendCommand(ILI9488_SLPOUT);
  delay(ILI9488_SLPOUT_DELAY);
}

void Arduino_ILI9488_18bit::displayOff(void)
{
  _bus->sendCommand(ILI9488_SLPIN);
  delay(ILI9488_SLPIN_DELAY);
}
