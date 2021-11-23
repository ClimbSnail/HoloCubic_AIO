#include "Arduino_HX8357A.h"

Arduino_HX8357A::Arduino_HX8357A(Arduino_DataBus *bus, int8_t rst, uint8_t r, bool ips)
    : Arduino_TFT(bus, rst, r, ips, HX8357A_TFTWIDTH, HX8357A_TFTHEIGHT, 0, 0, 0, 0)
{
}

void Arduino_HX8357A::begin(int32_t speed)
{
  Arduino_TFT::begin(speed);
}

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Arduino_HX8357A::setRotation(uint8_t r)
{
  Arduino_TFT::setRotation(r);
  switch (_rotation)
  {
  case 3:
    // TODO: not working
    r = (HX8357A_MADCTL_MY | HX8357A_MADCTL_MV | HX8357A_MADCTL_BGR);
    break;
  case 2:
    r = (HX8357A_MADCTL_MX | HX8357A_MADCTL_BGR);
    break;
  case 1:
    // TODO: not working
    r = (HX8357A_MADCTL_MX | HX8357A_MADCTL_MV | HX8357A_MADCTL_BGR);
    break;
  default: // case 0:
    r = (HX8357A_MADCTL_MY | HX8357A_MADCTL_BGR);
    break;
  }

  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_MEMORY_ACCESS_CONTROL, r);
  _bus->endWrite();
}

void Arduino_HX8357A::writeAddrWindow(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  if ((x != _currentX) || (w != _currentW))
  {
    _currentX = x;
    _currentW = w;
    x += _xStart;
    _data16.value = x;
    _bus->writeC8D8(HX8357A_COLUMN_ADDRESS_COUNTER_2, _data16.msb);
    _bus->writeC8D8(HX8357A_COLUMN_ADDRESS_COUNTER_1, _data16.lsb);
    _bus->writeC8D8(HX8357A_COLUMN_ADDRESS_START_2, _data16.msb);
    _bus->writeC8D8(HX8357A_COLUMN_ADDRESS_START_1, _data16.lsb);
    _data16.value = x + w - 1;
    _bus->writeC8D8(HX8357A_COLUMN_ADDRESS_END_2, _data16.msb);
    _bus->writeC8D8(HX8357A_COLUMN_ADDRESS_END_1, _data16.lsb);
  }

  if ((y != _currentY) || (h != _currentH))
  {
    _currentY = y;
    _currentH = h;
    y += _yStart;
    _data16.value = y;
    _bus->writeC8D8(HX8357A_ROW_ADDRESS_COUNTER_2, _data16.msb);
    _bus->writeC8D8(HX8357A_ROW_ADDRESS_COUNTER_1, _data16.lsb);
    _bus->writeC8D8(HX8357A_ROW_ADDRESS_START_2, _data16.msb);
    _bus->writeC8D8(HX8357A_ROW_ADDRESS_START_1, _data16.lsb);
    _data16.value = y + h - 1;
    _bus->writeC8D8(HX8357A_ROW_ADDRESS_END_2, _data16.msb);
    _bus->writeC8D8(HX8357A_ROW_ADDRESS_END_1, _data16.lsb);
  }

  _bus->writeCommand(HX8357A_SRAM_CONTROL);
}

void Arduino_HX8357A::invertDisplay(bool i)
{
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_DISPLAY_MODE_CONTROL, (_ips ^ i) ? HX8359A_INV_ON : HX8359A_INV_OFF);
  _bus->endWrite();
}

void Arduino_HX8357A::displayOn(void)
{
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_OSC_CONTROL_2, 0x01); // OSC_EN=1
  _bus->endWrite();
  delay(5);
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0x88);      // PON=0, DK=1
  _bus->writeC8D8(HX8357A_DISPLAY_MODE_CONTROL, 0x00); // DP_STB=00
  _bus->writeC8D8(HX8357A_POWER_CONTROL_4, 0x03);      // AP=011
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0x80);      // DK=0
  _bus->endWrite();
  delay(3);
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0x90); // PON=1
  _bus->endWrite();
  delay(3);
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0xD0);   // VCOMG=1
  _bus->writeC8D8(HX8357A_DISPLAY_CONTROL_3, 0x3C); // GON=1, DTE=1, D[1:0]=11
  _bus->endWrite();
}

void Arduino_HX8357A::displayOff(void)
{
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_DISPLAY_CONTROL_3, 0x34); // GON=1, DTE=1, D[1:0]=01
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0x90);   // VCOMG=0
  _bus->endWrite();
  delay(1);
  _bus->beginWrite();
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0x88); // PON=0, DK=1
  _bus->writeC8D8(HX8357A_POWER_CONTROL_4, 0x00); // AP=000
  _bus->writeC8D8(HX8357A_POWER_CONTROL_1, 0x89); // STB=1
  _bus->endWrite();
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_HX8357A::tftInit()
{
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(HX8357A_RST_DELAY);
    digitalWrite(_rst, HIGH);
    delay(HX8357A_RST_DELAY);
  }
  else
  {
    // Software Rest
  }

  uint8_t hx8357a_init_operations[] = {
      BEGIN_WRITE,
      WRITE_C8_D8, HX8357A_PAGE_SELECT, 0x00, // Command page 0
      END_WRITE,
      DELAY, 15,
      BEGIN_WRITE,
      WRITE_C8_D8, HX8357A_POWER_CONTROL_6, 0x04, // VGH VGL VCL  DDVDH
      WRITE_C8_D8, HX8357A_POWER_CONTROL_5, 0x1C,
      // Power Settng
      WRITE_C8_D8, HX8357A_VCOM_CONTROL_1, 0x94, // Set VCOM offset, VMF=0x52
      WRITE_C8_D8, HX8357A_VCOM_CONTROL_2, 0x69, // Set VCOMH voltage, VHH=0x64
      WRITE_C8_D8, HX8357A_VCOM_CONTROL_3, 0x63, // Set VCOML voltage, VML=0x71
      WRITE_C8_D8, HX8357A_OSC_CONTROL_2, 0x01,
      END_WRITE,
      DELAY, 10,
      BEGIN_WRITE,
      WRITE_C8_D8, HX8357A_POWER_CONTROL_6, 0x00,      // VGH VGL VCL  DDVDH
      WRITE_C8_D8, HX8357A_POWER_CONTROL_1, 0x8A,      //
      WRITE_C8_D8, HX8357A_DISPLAY_MODE_CONTROL, 0x00, //
      WRITE_C8_D8, HX8357A_POWER_CONTROL_4, 0x05,      //
      WRITE_C8_D8, HX8357A_POWER_CONTROL_1, 0x82,      //
      END_WRITE,
      DELAY, 10,
      BEGIN_WRITE,
      WRITE_C8_D8, HX8357A_POWER_CONTROL_1, 0x92, //
      END_WRITE,
      DELAY, 10,
      BEGIN_WRITE,
      WRITE_C8_D8, HX8357A_POWER_CONTROL_1, 0xD4, //

      WRITE_C8_D8, HX8357A_COLMOD, 0x55,          // 16-bit/pixel
      WRITE_C8_D8, HX8357A_OSC_CONTROL_1, 0x21,   // Fosc=130%*5.2MHZ   21
      WRITE_C8_D8, HX8357A_POWER_CONTROL_3, 0x00, // FS0[1:0]=01, Set the operating frequency of the step-up circuit 1
      WRITE_C8_D8, HX8357A_POWER_CONTROL_2, 0x00,
      WRITE_C8_D8, HX8357A_DISPLAY_CONTROL_1, 0x33,
      WRITE_C8_D8, HX8357A_DISPLAY_CONTROL_2, 0x01,
      WRITE_C8_D8, HX8357A_FRAME_RATE_CONTROL_1, 0x00,
      WRITE_C8_D8, HX8357A_FRAME_RATE_CONTROL_2, 0x00,
      WRITE_C8_D8, HX8357A_FRAME_RATE_CONTROL_3, 0x01, // 0A
      WRITE_C8_D8, HX8357A_FRAME_RATE_CONTROL_4, 0x0A,
      WRITE_C8_D8, HX8357A_CYCLE_CONTROL_2, 0x20,
      WRITE_C8_D8, HX8357A_CYCLE_CONTROL_3, 0xA3,
      WRITE_C8_D8, HX8357A_DISPLAY_INVERSION, 0x00, // 00 100416

      WRITE_C8_D8, HX8357A_RGB_INTERFACE_CONTROL_1, 0x00, // RGB MODE1  0X03=RGB MODE2
      WRITE_C8_D8, HX8357A_RGB_INTERFACE_CONTROL_2, 0x00,
      WRITE_C8_D8, HX8357A_RGB_INTERFACE_CONTROL_3, 0x08,
      WRITE_C8_D8, HX8357A_RGB_INTERFACE_CONTROL_4, 0x08,
      WRITE_C8_D8, HX8357A_PANEL_CHARACTERIC, 0x02, // REV

      // Gamma
      WRITE_C8_D8, HX8357A_R1_CONTROL_1, 0x01,
      WRITE_C8_D8, HX8357A_R1_CONTROL_2, 0x0F,
      WRITE_C8_D8, HX8357A_R1_CONTROL_3, 0x0F,
      WRITE_C8_D8, HX8357A_R1_CONTROL_4, 0x26,
      WRITE_C8_D8, HX8357A_R1_CONTROL_5, 0x2C,
      WRITE_C8_D8, HX8357A_R1_CONTROL_6, 0x3C,
      WRITE_C8_D8, HX8357A_R1_CONTROL_7, 0x0B,
      WRITE_C8_D8, HX8357A_R1_CONTROL_8, 0x5F,
      WRITE_C8_D8, HX8357A_R1_CONTROL_9, 0x00,
      WRITE_C8_D8, HX8357A_R1_CONTROL_10, 0x06,
      WRITE_C8_D8, HX8357A_R1_CONTROL_11, 0x09,
      WRITE_C8_D8, HX8357A_R1_CONTROL_12, 0x0E,
      WRITE_C8_D8, HX8357A_R1_CONTROL_13, 0x16,

      WRITE_C8_D8, HX8357A_R1_CONTROL_18, 0x01,
      WRITE_C8_D8, HX8357A_R1_CONTROL_19, 0x1D,
      WRITE_C8_D8, HX8357A_R1_CONTROL_20, 0x21,
      WRITE_C8_D8, HX8357A_R1_CONTROL_21, 0x3A,
      WRITE_C8_D8, HX8357A_R1_CONTROL_22, 0x39,
      WRITE_C8_D8, HX8357A_R1_CONTROL_23, 0x3F,
      WRITE_C8_D8, HX8357A_R1_CONTROL_24, 0x2D,
      WRITE_C8_D8, HX8357A_R1_CONTROL_25, 0x7F,
      WRITE_C8_D8, HX8357A_R1_CONTROL_26, 0x02,
      WRITE_C8_D8, HX8357A_R1_CONTROL_27, 0x15,
      WRITE_C8_D8, HX8357A_R1_CONTROL_28, 0x1B,
      WRITE_C8_D8, HX8357A_R1_CONTROL_29, 0x1B,
      WRITE_C8_D8, HX8357A_R1_CONTROL_30, 0x1A,
      WRITE_C8_D8, HX8357A_R1_CONTROL_35, 0x55,

      // #if 1

      WRITE_C8_D8, 0xff, 0x01,
      WRITE_C8_D8, 0x00, 0x01,
      WRITE_C8_D8, 0x01, 0x00,
      WRITE_C8_D8, 0x02, 0x01,
      WRITE_C8_D8, 0x03, 0x03,
      WRITE_C8_D8, 0x04, 0x05,
      WRITE_C8_D8, 0x05, 0x06,
      WRITE_C8_D8, 0x06, 0x08,
      WRITE_C8_D8, 0x07, 0x0C,
      WRITE_C8_D8, 0x08, 0x0E,
      WRITE_C8_D8, 0x09, 0x11,
      WRITE_C8_D8, 0x0A, 0x12,
      WRITE_C8_D8, 0x0B, 0x14,
      WRITE_C8_D8, 0x0C, 0x1B,
      WRITE_C8_D8, 0x0D, 0x23,
      WRITE_C8_D8, 0x0E, 0x29,
      WRITE_C8_D8, 0x0F, 0x2F,
      WRITE_C8_D8, 0x10, 0x34,
      WRITE_C8_D8, 0x11, 0x39,
      WRITE_C8_D8, 0x12, 0x3E,
      WRITE_C8_D8, 0x13, 0x43,
      WRITE_C8_D8, 0x14, 0x48,
      WRITE_C8_D8, 0x15, 0x4C,
      WRITE_C8_D8, 0x16, 0x51,
      WRITE_C8_D8, 0x17, 0x55,
      WRITE_C8_D8, 0x18, 0x59,
      WRITE_C8_D8, 0x19, 0x5D,
      WRITE_C8_D8, 0x1A, 0x60,
      WRITE_C8_D8, 0x1B, 0x64,
      WRITE_C8_D8, 0x1C, 0x68,
      WRITE_C8_D8, 0x1D, 0x6C,
      WRITE_C8_D8, 0x1E, 0x70,
      WRITE_C8_D8, 0x1F, 0x73,
      WRITE_C8_D8, 0x20, 0x77,
      WRITE_C8_D8, 0x21, 0x7B,
      WRITE_C8_D8, 0x22, 0x7F,
      WRITE_C8_D8, 0x23, 0x83,
      WRITE_C8_D8, 0x24, 0x87,
      WRITE_C8_D8, 0x25, 0x8A,
      WRITE_C8_D8, 0x26, 0x8E,
      WRITE_C8_D8, 0x27, 0x92,
      WRITE_C8_D8, 0x28, 0x96,
      WRITE_C8_D8, 0x29, 0x9A,
      WRITE_C8_D8, 0x2A, 0x9F,
      WRITE_C8_D8, 0x2B, 0xA3,
      WRITE_C8_D8, 0x2C, 0xA7,
      WRITE_C8_D8, 0x2D, 0xAC,
      WRITE_C8_D8, 0x2E, 0xAF,
      WRITE_C8_D8, 0x2F, 0xB3,
      WRITE_C8_D8, 0x30, 0xB7,
      WRITE_C8_D8, 0x31, 0xBA,
      WRITE_C8_D8, 0x32, 0xBE,
      WRITE_C8_D8, 0x33, 0xC3,
      WRITE_C8_D8, 0x34, 0xC7,
      WRITE_C8_D8, 0x35, 0xCC,
      WRITE_C8_D8, 0x36, 0xD1,
      WRITE_C8_D8, 0x37, 0xD7,
      WRITE_C8_D8, 0x38, 0xDD,
      WRITE_C8_D8, 0x39, 0xE3,
      WRITE_C8_D8, 0x3A, 0xE8,
      WRITE_C8_D8, 0x3B, 0xEA,
      WRITE_C8_D8, 0x3C, 0xEC,
      WRITE_C8_D8, 0x3D, 0xEF,
      WRITE_C8_D8, 0x3E, 0xF3,
      WRITE_C8_D8, 0x3F, 0xF6,
      WRITE_C8_D8, 0x40, 0xFE,
      WRITE_C8_D8, 0x41, 0x00,
      WRITE_C8_D8, 0x42, 0x01,
      WRITE_C8_D8, 0x43, 0x03,
      WRITE_C8_D8, 0x44, 0x05,
      WRITE_C8_D8, 0x45, 0x06,
      WRITE_C8_D8, 0x46, 0x08,
      WRITE_C8_D8, 0x47, 0x0C,
      WRITE_C8_D8, 0x48, 0x0E,
      WRITE_C8_D8, 0x49, 0x11,
      WRITE_C8_D8, 0x4A, 0x12,
      WRITE_C8_D8, 0x4B, 0x14,
      WRITE_C8_D8, 0x4C, 0x1B,
      WRITE_C8_D8, 0x4D, 0x23,
      WRITE_C8_D8, 0x4E, 0x29,
      WRITE_C8_D8, 0x4F, 0x2F,
      WRITE_C8_D8, 0x50, 0x34,
      WRITE_C8_D8, 0x51, 0x39,
      WRITE_C8_D8, 0x52, 0x3E,
      WRITE_C8_D8, 0x53, 0x43,
      WRITE_C8_D8, 0x54, 0x48,
      WRITE_C8_D8, 0x55, 0x4C,
      WRITE_C8_D8, 0x56, 0x51,
      WRITE_C8_D8, 0x57, 0x55,
      WRITE_C8_D8, 0x58, 0x59,
      WRITE_C8_D8, 0x59, 0x5D,
      WRITE_C8_D8, 0x5A, 0x60,
      WRITE_C8_D8, 0x5B, 0x64,
      WRITE_C8_D8, 0x5C, 0x68,
      WRITE_C8_D8, 0x5D, 0x6C,
      WRITE_C8_D8, 0x5E, 0x70,
      WRITE_C8_D8, 0x5F, 0x73,
      WRITE_C8_D8, 0x60, 0x77,
      WRITE_C8_D8, 0x61, 0x7B,
      WRITE_C8_D8, 0x62, 0x7F,
      WRITE_C8_D8, 0x63, 0x83,
      WRITE_C8_D8, 0x64, 0x87,
      WRITE_C8_D8, 0x65, 0x8A,
      WRITE_C8_D8, 0x66, 0x8E,
      WRITE_C8_D8, 0x67, 0x92,
      WRITE_C8_D8, 0x68, 0x96,
      WRITE_C8_D8, 0x69, 0x9A,
      WRITE_C8_D8, 0x6A, 0x9F,
      WRITE_C8_D8, 0x6B, 0xA3,
      WRITE_C8_D8, 0x6C, 0xA7,
      WRITE_C8_D8, 0x6D, 0xAC,
      WRITE_C8_D8, 0x6E, 0xAF,
      WRITE_C8_D8, 0x6F, 0xB3,
      WRITE_C8_D8, 0x70, 0xB7,
      WRITE_C8_D8, 0x71, 0xBA,
      WRITE_C8_D8, 0x72, 0xBE,
      WRITE_C8_D8, 0x73, 0xC3,
      WRITE_C8_D8, 0x74, 0xC7,
      WRITE_C8_D8, 0x75, 0xCC,
      WRITE_C8_D8, 0x76, 0xD1,
      WRITE_C8_D8, 0x77, 0xD7,
      WRITE_C8_D8, 0x78, 0xDD,
      WRITE_C8_D8, 0x79, 0xE3,
      WRITE_C8_D8, 0x7A, 0xE8,
      WRITE_C8_D8, 0x7B, 0xEA,
      WRITE_C8_D8, 0x7C, 0xEC,
      WRITE_C8_D8, 0x7D, 0xEF,
      WRITE_C8_D8, 0x7E, 0xF3,
      WRITE_C8_D8, 0x7F, 0xF6,
      WRITE_C8_D8, 0x80, 0xFE,
      WRITE_C8_D8, 0x81, 0x00,
      WRITE_C8_D8, 0x82, 0x01,
      WRITE_C8_D8, 0x83, 0x03,
      WRITE_C8_D8, 0x84, 0x05,
      WRITE_C8_D8, 0x85, 0x06,
      WRITE_C8_D8, 0x86, 0x08,
      WRITE_C8_D8, 0x87, 0x0C,
      WRITE_C8_D8, 0x88, 0x0E,
      WRITE_C8_D8, 0x89, 0x11,
      WRITE_C8_D8, 0x8A, 0x12,
      WRITE_C8_D8, 0x8B, 0x14,
      WRITE_C8_D8, 0x8C, 0x1B,
      WRITE_C8_D8, 0x8D, 0x23,
      WRITE_C8_D8, 0x8E, 0x29,
      WRITE_C8_D8, 0x8F, 0x2F,
      WRITE_C8_D8, 0x90, 0x34,
      WRITE_C8_D8, 0x91, 0x39,
      WRITE_C8_D8, 0x92, 0x3E,
      WRITE_C8_D8, 0x93, 0x43,
      WRITE_C8_D8, 0x94, 0x48,
      WRITE_C8_D8, 0x95, 0x4C,
      WRITE_C8_D8, 0x96, 0x51,
      WRITE_C8_D8, 0x97, 0x55,
      WRITE_C8_D8, 0x98, 0x59,
      WRITE_C8_D8, 0x99, 0x5D,
      WRITE_C8_D8, 0x9A, 0x60,
      WRITE_C8_D8, 0x9B, 0x64,
      WRITE_C8_D8, 0x9C, 0x68,
      WRITE_C8_D8, 0x9D, 0x6C,
      WRITE_C8_D8, 0x9E, 0x70,
      WRITE_C8_D8, 0x9F, 0x73,
      WRITE_C8_D8, 0xA0, 0x77,
      WRITE_C8_D8, 0xA1, 0x7B,
      WRITE_C8_D8, 0xA2, 0x7F,
      WRITE_C8_D8, 0xA3, 0x83,
      WRITE_C8_D8, 0xA4, 0x87,
      WRITE_C8_D8, 0xA5, 0x8A,
      WRITE_C8_D8, 0xA6, 0x8E,
      WRITE_C8_D8, 0xA7, 0x92,
      WRITE_C8_D8, 0xA8, 0x96,
      WRITE_C8_D8, 0xA9, 0x9A,
      WRITE_C8_D8, 0xAA, 0x9F,
      WRITE_C8_D8, 0xAB, 0xA3,
      WRITE_C8_D8, 0xAC, 0xA7,
      WRITE_C8_D8, 0xAD, 0xAC,
      WRITE_C8_D8, 0xAE, 0xAF,
      WRITE_C8_D8, 0xAF, 0xB3,
      WRITE_C8_D8, 0xB0, 0xB7,
      WRITE_C8_D8, 0xB1, 0xBA,
      WRITE_C8_D8, 0xB2, 0xBE,
      WRITE_C8_D8, 0xB3, 0xC3,
      WRITE_C8_D8, 0xB4, 0xC7,
      WRITE_C8_D8, 0xB5, 0xCC,
      WRITE_C8_D8, 0xB6, 0xD1,
      WRITE_C8_D8, 0xB7, 0xD7,
      WRITE_C8_D8, 0xB8, 0xDD,
      WRITE_C8_D8, 0xB9, 0xE3,
      WRITE_C8_D8, 0xBA, 0xE8,
      WRITE_C8_D8, 0xBB, 0xEA,
      WRITE_C8_D8, 0xBC, 0xEC,
      WRITE_C8_D8, 0xBD, 0xEF,
      WRITE_C8_D8, 0xBE, 0xF3,
      WRITE_C8_D8, 0xBF, 0xF6,
      WRITE_C8_D8, 0xC0, 0xFE,

      WRITE_C8_D8, 0xff, 0x00,
      WRITE_C8_D8, HX8357A_TE_CONTROL, 0x08,
      WRITE_C8_D8, HX8357A_CENON_CONTROL, 0x00,
      WRITE_C8_D8, HX8357A_POWER_SAVING_COUNTER_1, 0x1F, //EQVCI_M1=0x00
      WRITE_C8_D8, HX8357A_POWER_SAVING_COUNTER_2, 0x1F, //EQGND_M1=0x1C
      WRITE_C8_D8, HX8357A_POWER_SAVING_COUNTER_3, 0x20, //EQVCI_M0=0x1C
      WRITE_C8_D8, HX8357A_POWER_SAVING_COUNTER_4, 0x00, //EQGND_M0=0x1C
      WRITE_C8_D8, HX8357A_OPON_CONTROL_1, 0xD1,
      WRITE_C8_D8, HX8357A_OPON_CONTROL_2, 0xC0,
      WRITE_C8_D8, HX8357A_DISPLAY_CONTROL_3, 0x38,
      END_WRITE,
      DELAY, 80,
      BEGIN_WRITE,
      WRITE_C8_D8, HX8357A_DISPLAY_CONTROL_3, 0x3C, // GON=1, DTE=1, D[1:0]=11
      END_WRITE,
      DELAY, 100};

  _bus->batchOperation(hx8357a_init_operations, sizeof(hx8357a_init_operations));
  if (_ips)
  {
    _bus->sendCommand(HX8357A_DISPLAY_MODE_CONTROL);
    _bus->sendData(HX8359A_INV_ON);
  }
}
