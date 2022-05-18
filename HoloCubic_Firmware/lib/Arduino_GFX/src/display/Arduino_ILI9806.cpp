#include "Arduino_ILI9806.h"
#include "SPI.h"

Arduino_ILI9806::Arduino_ILI9806(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT(bus, rst, r, ips, ILI9806_TFTWIDTH, ILI9806_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_ILI9806::begin(int32_t speed)
{
  _override_datamode = SPI_MODE0; // always use SPI_MODE0
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ILI9806::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 3:
    r = (ILI9806_MADCTL_MY | ILI9806_MADCTL_MV | ILI9806_MADCTL_RGB);
    break;
  case 2:
    r = (ILI9806_MADCTL_MX | ILI9806_MADCTL_MY | ILI9806_MADCTL_RGB);
    break;
  case 1:
    r = (ILI9806_MADCTL_MX | ILI9806_MADCTL_MV | ILI9806_MADCTL_RGB);
    break;
  default: // case 0:
    r = ILI9806_MADCTL_RGB;
    break;
  }
  _bus->beginWrite();
  _bus->writeC8D8(ILI9806_MADCTL, r);
  _bus->endWrite();
}

void Arduino_ILI9806::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _bus->writeC8D16D16(ILI9806_CASET, x, x + w - 1);
  }

  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    _bus->writeC8D16D16(ILI9806_PASET, y, y + h - 1);
  }

  _bus->writeCommand(ILI9806_RAMWR); // write to RAM
}

void Arduino_ILI9806::invertDisplay(bool i)
{
  _bus->sendCommand((_ips ^ i) ? ILI9806_INVON : ILI9806_INVOFF);
}

void Arduino_ILI9806::displayOn(void)
{
  _bus->sendCommand(ILI9806_SLPOUT);
  delay(ILI9806_SLPOUT_DELAY);
  _bus->sendCommand(ILI9806_DISPON);
}

void Arduino_ILI9806::displayOff(void)
{
  _bus->sendCommand(ILI9806_DISPOFF);
  delay(10);
  _bus->sendCommand(ILI9806_SLPIN);
  delay(ILI9806_SLPIN_DELAY);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ILI9806::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(1);
    digitalWrite(_rst, LOW);
    delay(10);
    digitalWrite(_rst, HIGH);
    delay(ILI9806_RST_DELAY);
  }
  else
  {
    // Software Rest
    _bus->sendCommand(ILI9806_SWRESET);
    delay(ILI9806_RST_DELAY);
  }

  uint8_t ili9806_init_operations[] = {
      BEGIN_WRITE,
      WRITE_COMMAND_8, 0xFF, // EXTC Command Set enable register
      WRITE_BYTES, 3, 0xFF, 0x98, 0x06,

      WRITE_C8_D8, 0xBA, 0x60, // SPI Interface Setting

      WRITE_COMMAND_8, 0xBC, // GIP 1
      WRITE_BYTES, 21,
      0x01, 0x10, 0x00, 0x00, 0x01,
      0x01, 0x0B, 0x11, 0x32, 0x10,
      0x00, 0x00, 0x01, 0x01, 0x01,
      0x01, 0x50, 0x52, 0x01, 0x00,
      0x40,

      WRITE_COMMAND_8, 0xBD, // GIP 2
      WRITE_BYTES, 8,
      0x01, 0x23, 0x45, 0x67, 0x01,
      0x23, 0x45, 0x67,

      WRITE_COMMAND_8, 0xBE, // GIP 3
      WRITE_BYTES, 9,
      0x00, 0x21, 0xAB, 0x60, 0x22,
      0x22, 0x22, 0x22, 0x22,

      WRITE_C8_D8, 0xC7, 0x30, // VCOM Control

      WRITE_COMMAND_8, 0xED, // EN_volt_reg
      WRITE_BYTES, 3, 0x7F, 0x0F, 0x00,

      WRITE_COMMAND_8, 0xC0,            // Power Control 1
      WRITE_BYTES, 3, 0x03, 0x0B, 0x0C, // 0A VGH VGL

      WRITE_C8_D16, 0xFD, 0x0A, 0x00, // External Power Selection Set

      WRITE_C8_D8, 0xFC, 0x08, // LVGL

      WRITE_COMMAND_8, 0xDF, // Engineering Setting
      WRITE_BYTES, 6,
      0x00, 0x00, 0x00, 0x00, 0x00,
      0x20,

      WRITE_C8_D8, 0xF3, 0x74, // DVDD Voltage Setting

      WRITE_COMMAND_8, 0xB4, // Display Inversion Control
      WRITE_BYTES, 3, 0x00, 0x00, 0x00,

      WRITE_C8_D16, 0xB5, 0x08, 0x15, // Blanking Porch Control

      WRITE_C8_D8, 0xF7, 0x81, // 480x854

      WRITE_COMMAND_8, 0xB1, // Frame Rate Control
      WRITE_BYTES, 3, 0x00, 0x13, 0x13,

      WRITE_COMMAND_8, 0xF2, //Panel Timing Control
      WRITE_BYTES, 4, 0x80, 0x04, 0x40, 0x28,

      WRITE_COMMAND_8, 0xC1, //Power Control 2
      WRITE_BYTES, 3,
      0x17,
      0x71, //VGMP
      0x71, //VGMN

      WRITE_COMMAND_8, 0xE0, //P_Gamma
      WRITE_BYTES, 16,
      0x00, //P1
      0x13, //P2
      0x1A, //P3
      0x0C, //P4
      0x0E, //P5
      0x0B, //P6
      0x07, //P7
      0x05, //P8
      0x05, //P9
      0x0A, //P10
      0x0F, //P11
      0x0F, //P12
      0x0E, //P13
      0x1C, //P14
      0x16, //P15
      0x00, //P16

      WRITE_COMMAND_8, 0xE1, //N_Gamma
      WRITE_BYTES, 16,
      0x00, //P1
      0x13, //P2
      0x1A, //P3
      0x0C, //P4
      0x0E, //P5
      0x0B, //P6
      0x07, //P7
      0x05, //P8
      0x05, //P9
      0x0A, //P10
      0x0F, //P11
      0x0F, //P12
      0x0E, //P13
      0x1C, //P14
      0x16, //P15
      0x00, //P16

      WRITE_C8_D8, 0x3A, 0x55, //55-16BIT,66-18BIT,77-24BIT

      WRITE_COMMAND_8, 0x11,
      END_WRITE,

      DELAY, ILI9806_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, 0x29,
      END_WRITE,

      DELAY, 25,

      BEGIN_WRITE,
      WRITE_COMMAND_8, 0x2C,
      END_WRITE};

  _bus->batchOperation(ili9806_init_operations, sizeof(ili9806_init_operations));

  if (_ips)
  {
    _bus->sendCommand(ILI9806_INVON);
  }
}
