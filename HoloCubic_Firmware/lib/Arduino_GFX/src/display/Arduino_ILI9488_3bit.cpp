#include "Arduino_DataBus.h"
#if !defined(LITTLE_FOOT_PRINT)

#include "Arduino_ILI9488_3bit.h"

Arduino_ILI9488_3bit::Arduino_ILI9488_3bit(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_G(ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT), _bus(bus), _rst(rst), _rotation(r), _ips(ips)
{
}

void Arduino_ILI9488_3bit::begin(int32_t speed)
{
  _bus->begin(speed);

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
  else
  {
    _bus->sendCommand(ILI9488_SWRESET); // 1: Software reset
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

      WRITE_C8_D8, 0x3A, 0x01, // Interface Pixel Format, 3 bit

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

  uint16_t r;
  // setRotation
  switch (_rotation)
  {
  case 3:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MV);
    break;
  case 2:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MX);
    break;
  case 1:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MV | ILI9488_MADCTL_MX | ILI9488_MADCTL_MY);
    break;
  default: // case 0:
    r = (ILI9488_MADCTL_BGR | ILI9488_MADCTL_MY);
    break;
  }
  _bus->beginWrite();
  _bus->writeC8D8(ILI9488_MADCTL, r);
  _bus->endWrite();
}

void Arduino_ILI9488_3bit::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
{
  printf("Not Implemented drawBitmap()");
  UNUSED(x);
  UNUSED(y);
  UNUSED(bitmap);
  UNUSED(w);
  UNUSED(h);
  UNUSED(color);
  UNUSED(bg);
}

void Arduino_ILI9488_3bit::drawIndexedBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint16_t *color_index, int16_t w, int16_t h)
{
  printf("Not Implemented drawIndexedBitmap()");
  UNUSED(x);
  UNUSED(y);
  UNUSED(bitmap);
  UNUSED(color_index);
  UNUSED(w);
  UNUSED(h);
}

void Arduino_ILI9488_3bit::draw3bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h)
{
  UNUSED(x);
  UNUSED(y);
  _bus->beginWrite();
  writeAddrWindow(0, 0, w, h);
  _bus->writeBytes(bitmap, w * h / 2);
  _bus->endWrite();
}

void Arduino_ILI9488_3bit::draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
  printf("Not Implemented draw16bitRGBBitmap()");
  UNUSED(x);
  UNUSED(y);
  UNUSED(bitmap);
  UNUSED(w);
  UNUSED(h);
}

void Arduino_ILI9488_3bit::draw24bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h)
{
  printf("Not Implemented draw24bitRGBBitmap()");
  UNUSED(x);
  UNUSED(y);
  UNUSED(bitmap);
  UNUSED(w);
  UNUSED(h);
}

void Arduino_ILI9488_3bit::invertDisplay(bool i)
{
  _bus->sendCommand(i ? ILI9488_INVON : ILI9488_INVOFF);
}

void Arduino_ILI9488_3bit::displayOn(void)
{
  _bus->sendCommand(ILI9488_SLPOUT);
  delay(ILI9488_SLPOUT_DELAY);
}

void Arduino_ILI9488_3bit::displayOff(void)
{
  _bus->sendCommand(ILI9488_SLPIN);
  delay(ILI9488_SLPIN_DELAY);
}

void Arduino_ILI9488_3bit::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  _bus->writeC8D16D16(ILI9488_CASET, x, x + w - 1);
  _bus->writeC8D16D16(ILI9488_PASET, y, y + h - 1);
  _bus->writeCommand(ILI9488_RAMWR);
}

#endif // !defined(LITTLE_FOOT_PRINT)
