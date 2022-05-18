#include "Arduino_HX8357B.h"

Arduino_HX8357B::Arduino_HX8357B(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips /* = false */)
    : Arduino_TFT(bus, rst, r, ips, HX8357B_TFTWIDTH, HX8357B_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_HX8357B::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_HX8357B::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 3:
    r = (HX8357B_MADCTL_MX | HX8357B_MADCTL_MY | HX8357B_MADCTL_MV | HX8357B_MADCTL_BGR);
    break;
  case 2:
    r = (HX8357B_MADCTL_MY | HX8357B_MADCTL_BGR);
    break;
  case 1:
    r = (HX8357B_MADCTL_MV | HX8357B_MADCTL_BGR);
    break;
  default: // case 0:
    r = (HX8357B_MADCTL_MX | HX8357B_MADCTL_BGR);
    break;
  }
  _bus->beginWrite();
  _bus->writeC8D8(HX8357B_SET_ADDRESS_MODE, r);
  _bus->endWrite();
}

void Arduino_HX8357B::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _bus->writeC8D16D16(HX8357B_SET_COLUMN_ADDRESS, x, x + w - 1);
  }

  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    _bus->writeC8D16D16(HX8357B_SET_PAGE_ADDRESS, y, y + h - 1);
  }

  _bus->writeCommand(HX8357B_WRITE_MEMORY_START); // write to RAM
}

void Arduino_HX8357B::invertDisplay(bool i)
{
  _bus->sendCommand((_ips ^ i) ? HX8357B_ENTER_INVERSION_MODE : HX8357B_EXIT_INVERSION_MODE);
}

void Arduino_HX8357B::displayOn(void)
{
  _bus->sendCommand(HX8357B_EXIT_SLEEP_MODE);
  delay(HX8357B_SLPOUT_DELAY);
}

void Arduino_HX8357B::displayOff(void)
{
  _bus->sendCommand(HX8357B_ENTER_SLEEP_MODE);
  delay(HX8357B_SLPIN_DELAY);
}

void Arduino_HX8357B::tftInit()
{
  if (_rst != GFX_NOT_DEFINED)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(HX8357B_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(HX8357B_RST_DELAY);
  }
  else
  {
    // Software Rest
    _bus->sendCommand(HX8357B_SOFTWARE_RESET);
    delay(HX8357B_RST_DELAY);
  }

  //************* Start Initial Sequence **********//
  uint8_t hx8357b_init_operations[] = {
      BEGIN_WRITE,
      WRITE_COMMAND_8, HX8357B_EXIT_SLEEP_MODE,
      END_WRITE,

      DELAY, HX8357B_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, HX8357B_SET_EP_FUNCTION,
      WRITE_BYTES, 4, 0x02, 0x01, 0x02, 0x01,

      WRITE_C8_D8, HX8357B_SET_DISPLAY_MODE, 0x00, // Set RM, DM

      WRITE_COMMAND_8, HX8357B_SET_PANEL_DRIVING,
      WRITE_BYTES, 5,
      0x10, // REV SM GS
      0x3B, // NL[5:0]
      0x00, // SCN[6:0]
      0x02, // NDL 0 PTS[2:0]
      0x11, // PTG ISC[3:0]

      WRITE_C8_D8, HX8357B_SET_DISPLAY_TIMING_NORMAL, 0x10, // ne inversion

      WRITE_COMMAND_8, HX8357B_SET_GAMMA,
      WRITE_BYTES, 12,
      0x00, // KP1,KP0
      0x32, // KP3,KP2
      0x36, // KP5,KP4
      0x45, // RP1,RP0
      0x06, // VRP0  01
      0x16, // VRP1
      0x37, // KN1,KN0
      0x75, // KN3,KN2
      0x77, // KN5,KN4
      0x54, // RN1,RN0
      0x0c, // VRN0
      0x00, // VRN1  01

      WRITE_COMMAND_8, HX8357B_SET_POWER,
      WRITE_BYTES, 3,
      0x44, // DDVDH :5.28
      0x42, // BT VGH:15.84    VGL:-7.92
      0x06, // VREG1  4.625V

      WRITE_C8_D16, HX8357B_SET_VCOM, 0x43, 0x16,         // VCOMH
      WRITE_C8_D16, HX8357B_SET_POWER_NORMAL, 0x04, 0x22, // 12
      WRITE_C8_D16, HX8357B_SET_POWER_PARTIAL, 0x04, 0x12,
      WRITE_C8_D16, HX8357B_SET_POWER_IDLE, 0x07, 0x12,
      WRITE_C8_D8, HX8357B_SET_PANEL_RELATED, 0x00,

      WRITE_C8_D8, HX8357B_SET_DISPLAY_FRAME, 0x08, // 61.51Hz

      WRITE_C8_D8, HX8357B_SET_PIXEL_FORMAT, 0X55,
      END_WRITE,

      DELAY, HX8357B_SLPOUT_DELAY,

      BEGIN_WRITE,
      WRITE_COMMAND_8, HX8357B_SET_DISPLAY_ON, //Display On
      END_WRITE,

      DELAY, 50};

  _bus->batchOperation(hx8357b_init_operations, sizeof(hx8357b_init_operations));

  if (_ips)
  {
    _bus->sendCommand(HX8357B_ENTER_INVERSION_MODE);
  }
}
