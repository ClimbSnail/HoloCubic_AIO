/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_ST7735.h"

Arduino_ST7735::Arduino_ST7735(
    Arduino_DataBus *bus, int8_t rst, uint8_t r,
    bool ips, int16_t w, int16_t h,
    uint8_t col_offset1, uint8_t row_offset1, uint8_t col_offset2, uint8_t row_offset2,
    bool bgr)
    : Arduino_TFT(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2), _bgr(bgr)
{
}

void Arduino_ST7735::begin(int32_t speed)
{
#if defined(ESP8266) || defined(ESP32)
  if (speed == 0)
  {
    speed = 27000000; // ST7735 Maximum supported speed
  }
// Teensy 4.x
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)
  if (speed == 0)
  {
    speed = 27000000; // ST7735 Maximum supported speed
  }
#endif
  Arduino_TFT::begin(speed);
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ST7735::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(ST7735_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(ST7735_RST_DELAY);
  }
  // else
  // {
  // Software Rest
  _bus->sendCommand(ST7735_SWRESET); // 1: Software reset
  delay(ST7735_RST_DELAY);
  // }

  _bus->sendCommand(ST7735_SLPOUT); //  2: Out of sleep mode, no args, w/delay
  delay(ST7735_SLPOUT_DELAY);
  _bus->sendCommand(ST7735_COLMOD); // 3: Set color mode, 1 arg + delay:
  _bus->sendData(0x05);             // 16-bit color
  if (_ips)
  {
    _bus->sendCommand(ST7735_INVON);
  }

  uint8_t gamma[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, ST7735_GMCTRP1,  // Gamma Adjustments (pos. polarity), 16 args:
    WRITE_DATA_8, 16,
    0x09, 0x16, 0x09, 0x20,           // (Not entirely necessary, but provides
    0x21, 0x1B, 0x13, 0x19,           //  accurate colors)
    0x17, 0x15, 0x1E, 0x2B,
    0x04, 0x05, 0x02, 0x0E,
    END_WRITE,
    BEGIN_WRITE,
    WRITE_COMMAND_8, ST7735_GMCTRN1,  // Gamma Adjustments (neg. polarity), 16 args:
    WRITE_DATA_8, 16,
    0x0B, 0x14, 0x08, 0x1E,           // (Not entirely necessary, but provides
    0x22, 0x1D, 0x18, 0x1E,           //  accurate colors)
    0x1B, 0x1A, 0x24, 0x2B,
    0x06, 0x06, 0x02, 0x0F,
    END_WRITE,
    DELAY, 10
  };
  _bus->batchOperation(gamma, sizeof(gamma)); // 4: Gamma correction

  _bus->sendCommand(ST7735_NORON);  // 5: Normal display on, no args, w/delay
  delay(10);
  _bus->sendCommand(ST7735_DISPON); // 6: Main screen turn on, no args, w/delay
}

void Arduino_ST7735::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    int16_t x_start = x + _xStart, x_end = x + w - 1 + _xStart;

    _bus->writeCommand(ST7735_CASET); // Column addr set
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

    _bus->writeCommand(ST7735_RASET); // Row addr set
    _bus->write(y_start >> 8);
    _bus->write(y_start & 0xFF); // YSTART
    _bus->write(y_end >> 8);
    _bus->write(y_end & 0xFF); // YEND

    _currentY = y;
    _currentH = h;
  }

  _bus->writeCommand(ST7735_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_ST7735::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 1:
    r = ST7735_MADCTL_MY | ST7735_MADCTL_MV | (_bgr ? ST7735_MADCTL_BGR : ST7735_MADCTL_RGB);
    break;
  case 2:
    r = (_bgr ? ST7735_MADCTL_BGR : ST7735_MADCTL_RGB);
    break;
  case 3:
    r = ST7735_MADCTL_MX | ST7735_MADCTL_MV | (_bgr ? ST7735_MADCTL_BGR : ST7735_MADCTL_RGB);
    break;
  default: // case 0:
    r = ST7735_MADCTL_MX | ST7735_MADCTL_MY | (_bgr ? ST7735_MADCTL_BGR : ST7735_MADCTL_RGB);
    break;
  }
  _bus->beginWrite();
  _bus->writeCommand(ST7735_MADCTL);
  _bus->write(r);
  _bus->endWrite();
}

void Arduino_ST7735::invertDisplay(bool i)
{
  _bus->sendCommand(_ips ? (i ? ST7735_INVOFF : ST7735_INVON) : (i ? ST7735_INVON : ST7735_INVOFF));
}

void Arduino_ST7735::displayOn(void)
{
  _bus->sendCommand(ST7735_SLPOUT);
  delay(ST7735_SLPOUT_DELAY);
}

void Arduino_ST7735::displayOff(void)
{
  _bus->sendCommand(ST7735_SLPIN);
  delay(ST7735_SLPIN_DELAY);
}
